# Solar Tracker Web Dashboard

This is a local web dashboard for monitoring and controlling the solar tracker.

## How to use

1.  **Serve the dashboard:** Since this uses `fetch` to communicate with the tracker, it needs to be served via a web server (even if it's just local).
    You can use Python's built-in server:
    ```bash
    cd dashboard
    python3 -m http.server 8000
    ```
2.  **Open in browser:** Go to `http://localhost:8000`.
3.  **Connect:** Enter the IP address of your Arduino tracker in the input field and click "Connect".

## Features

- Real-time orientation data (Heading, Pitch, Tilt).
- Light sensor (LDR) visualization.
- System status and alerts (Fallen detection).
- Manual control pad.
- Automatic IP address persistence (using LocalStorage).

## CORS Note

The Arduino code in `tracker.ino` has been updated to include `Access-Control-Allow-Origin: *` headers to allow this dashboard to fetch data from it.
