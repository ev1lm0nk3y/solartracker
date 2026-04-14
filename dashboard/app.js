let trackerIp = localStorage.getItem('tracker-ip') || '';
let updateInterval = null;

const elements = {
    ipInput: document.getElementById('tracker-ip'),
    connectBtn: document.getElementById('connect-btn'),
    statusBadge: document.getElementById('connection-status'),
    heading: document.getElementById('heading-val'),
    pitch: document.getElementById('pitch-val'),
    tilt: document.getElementById('tilt-val'),
    mode: document.getElementById('mode-val'),
    rotStatus: document.getElementById('rot-status-val'),
    pitchStatus: document.getElementById('pitch-status-val'),
    fallenAlert: document.getElementById('fallen-alert'),
    lcd: [
        document.getElementById('lcd-0'),
        document.getElementById('lcd-1'),
        document.getElementById('lcd-2'),
        document.getElementById('lcd-3')
    ],
    ldr: {
        tl: document.querySelector('#ldr-tl .value'),
        tr: document.querySelector('#ldr-tr .value'),
        bl: document.querySelector('#ldr-bl .value'),
        br: document.querySelector('#ldr-br .value'),
        avgTop: document.getElementById('ldr-avg-top'),
        avgBottom: document.getElementById('ldr-avg-bottom'),
        avgLeft: document.getElementById('ldr-avg-left'),
        avgRight: document.getElementById('ldr-avg-right')
    }
};

// Mock State for Testing
let mockData = {
    heading: 180, pitch: 45, tilt: 1.5,
    manual: false, fallen: false,
    pitchStatus: 'STOP', rotationStatus: 'STOP',
    ldr: [512, 510, 520, 508],
    ldrAvg: { top: 511, bottom: 514, left: 516, right: 509 },
    lcd: [
        "L:512  R:514       ",
        "P:STOP R:STOP      ",
        "H:180  P:45        ",
        "T:25.5C WiFi:OK    "
    ]
};
let mockMovement = { heading: 0, pitch: 0 };

// Initialize
if (trackerIp) {
    elements.ipInput.value = trackerIp;
    startMonitoring();
}

elements.connectBtn.addEventListener('click', () => {
    trackerIp = elements.ipInput.value.trim();
    if (trackerIp) {
        localStorage.setItem('tracker-ip', trackerIp);
        startMonitoring();
    }
});

function startMonitoring() {
    if (updateInterval) clearInterval(updateInterval);
    
    updateData();
    updateInterval = setInterval(updateData, 1000);
}

async function updateData() {
    if (!trackerIp) return;

    // --- TEST MODE LOGIC ---
    if (trackerIp.toLowerCase() === 'test') {
        mockData.heading = (mockData.heading + mockMovement.heading + 360) % 360;
        mockData.pitch = Math.max(0, Math.min(90, mockData.pitch + mockMovement.pitch));
        
        if (!mockData.manual) {
            mockData.heading = (mockData.heading + 0.5) % 360; // Auto tracking simulation
            mockData.rotationStatus = 'CW'; // Simulate motor running while auto tracking
            mockData.pitchStatus = 'STOP';
        } else {
            mockData.rotationStatus = mockMovement.heading > 0 ? 'CW' : (mockMovement.heading < 0 ? 'CCW' : 'STOP');
            mockData.pitchStatus = mockMovement.pitch > 0 ? 'UP' : (mockMovement.pitch < 0 ? 'DOWN' : 'STOP');
        }

        mockData.ldr = mockData.ldr.map(val => {
            let newVal = val + Math.floor((Math.random() - 0.5) * 20);
            return Math.max(0, Math.min(1023, newVal)); // Jitter LDRs slightly
        });
        
        mockData.ldrAvg.top = Math.floor((mockData.ldr[0] + mockData.ldr[1]) / 2);
        mockData.ldrAvg.bottom = Math.floor((mockData.ldr[2] + mockData.ldr[3]) / 2);
        mockData.ldrAvg.left = Math.floor((mockData.ldr[0] + mockData.ldr[2]) / 2);
        mockData.ldrAvg.right = Math.floor((mockData.ldr[1] + mockData.ldr[3]) / 2);

        // Mock LCD dynamic text
        let start = mockData.manual ? 1 : 0;
        if (mockData.manual) mockData.lcd[0] = "=== MANUAL  MODE ===";
        
        mockData.lcd[start] = `L:${mockData.ldrAvg.left.toString().padEnd(4)} R:${mockData.ldrAvg.right.toString().padEnd(4)}    `.substring(0,20);
        mockData.lcd[start+1] = `P:${mockData.pitchStatus.padEnd(4)} R:${mockData.rotationStatus.padEnd(4)}    `.substring(0,20);
        mockData.lcd[start+2] = `H:${Math.floor(mockData.heading).toString().padEnd(3)}  P:${Math.floor(mockData.pitch).toString().padEnd(3)}     `.substring(0,20);
        if (start === 0) mockData.lcd[3] = "T:25.5C WiFi:OK    ";

        updateUI(mockData);
        elements.statusBadge.textContent = 'Test Mode';
        elements.statusBadge.className = 'status-badge test-mode';
        return;
    }
    // -----------------------

    try {
        const response = await fetch(`http://${trackerIp}/data`);
        if (!response.ok) throw new Error('Network response was not ok');
        
        const data = await response.json();
        updateUI(data);
        
        elements.statusBadge.textContent = 'Connected';
        elements.statusBadge.className = 'status-badge connected';
    } catch (error) {
        console.error('Fetch error:', error);
        elements.statusBadge.textContent = 'Disconnected';
        elements.statusBadge.className = 'status-badge disconnected';
    }
}

function updateUI(data) {
    elements.heading.textContent = data.heading.toFixed(1);
    elements.pitch.textContent = data.pitch.toFixed(1);
    elements.tilt.textContent = data.tilt.toFixed(1);
    
    elements.mode.textContent = data.manual ? 'MANUAL' : 'AUTO';
    elements.rotStatus.textContent = data.rotationStatus || 'STOP';
    elements.pitchStatus.textContent = data.pitchStatus || 'STOP';
    
    if (data.fallen) {
        elements.fallenAlert.classList.remove('hidden');
    } else {
        elements.fallenAlert.classList.add('hidden');
    }

    if (data.ldr && data.ldr.length === 4) {
        elements.ldr.tl.textContent = data.ldr[0];
        elements.ldr.tr.textContent = data.ldr[1];
        elements.ldr.bl.textContent = data.ldr[2];
        elements.ldr.br.textContent = data.ldr[3];
    }
    
    if (data.ldrAvg) {
        elements.ldr.avgTop.textContent = data.ldrAvg.top;
        elements.ldr.avgBottom.textContent = data.ldrAvg.bottom;
        elements.ldr.avgLeft.textContent = data.ldrAvg.left;
        elements.ldr.avgRight.textContent = data.ldrAvg.right;
    }

    if (data.lcd && data.lcd.length === 4) {
        data.lcd.forEach((line, i) => {
            elements.lcd[i].textContent = line;
        });
    }
}

// Controls
async function sendCommand(action) {
    if (!trackerIp) return;

    // --- TEST MODE LOGIC ---
    if (trackerIp.toLowerCase() === 'test') {
        console.log(`[Test Mode] Command: ${action}`);
        mockData.manual = true;
        mockMovement = { heading: 0, pitch: 0 };

        if (action === 'left') mockMovement.heading = -5;
        if (action === 'right') mockMovement.heading = 5;
        if (action === 'up') mockMovement.pitch = 5;
        if (action === 'down') mockMovement.pitch = -5;
        if (action === 'auto') mockData.manual = false;
        
        // 'stop' resets movement, which is already handled by the object reset above
        updateData(); // Instantly update UI for snappy feedback
        return;
    }
    // -----------------------

    try {
        await fetch(`http://${trackerIp}/cmd?action=${action}`);
    } catch (error) {
        console.error('Command error:', error);
    }
}

document.querySelectorAll('.control-btn').forEach(btn => {
    const action = btn.dataset.action;
    
    if (action === 'auto') {
        btn.addEventListener('click', () => sendCommand('auto'));
    } else {
        // For move buttons, stop on release
        btn.addEventListener('mousedown', () => sendCommand(action));
        btn.addEventListener('mouseup', () => sendCommand('stop'));
        btn.addEventListener('mouseleave', () => sendCommand('stop'));
        
        // Touch support
        btn.addEventListener('touchstart', (e) => {
            e.preventDefault();
            sendCommand(action);
        });
        btn.addEventListener('touchend', (e) => {
            e.preventDefault();
            sendCommand('stop');
        });
    }
});
