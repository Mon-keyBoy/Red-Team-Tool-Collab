const commandSocket = new WebSocket("ws://localhost:8080/command"); // WebSocket connection for sending commands to the C2 Server
const guiSocket = new WebSocket("ws://localhost:8080/gui"); // WebSocket connection for receiving responses from agents

commandSocket.onopen = () => console.log("[+] Connected to C2 server for commands."); // logs a message when the connection to the C2 server for commands is establisshed
commandSocket.onclose = () => console.log("[-] Command connection closed."); //  logs a message when the connection to the C2 server is closed

guiSocket.onopen = () => console.log("[+] Connected to C2 server for agent responses."); // Logs a message when the connection to the C2 server for agent responses is established.

// When a message is received from the C2 server for agent responses, the message is displayed in the output div.
guiSocket.onmessage = (event) => {
    const output = event.data;
    document.getElementById("output").innerHTML += `<b>Response:</b><br><pre>${output}</pre><br>`;
};

guiSocket.onclose = () => console.log("[-] GUI connection closed."); // Logs a message when the connection to the C2 server for agent responses is closed.

// Sends the command entered in the command box to the C2 server for commands, and displays the command in the output div.
function sendCommand() {
    const command = document.getElementById("command-box").value;
    if (command) {
        commandSocket.send(command);
        document.getElementById("output").innerHTML += `Command sent: <pre>${command}</pre><br>`;
        document.getElementById("command-box").value = "";
    }
}
