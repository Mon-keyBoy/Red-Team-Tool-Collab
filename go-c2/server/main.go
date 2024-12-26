package main

import (
	"fmt"
	"net/http"
	"sync"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

// WebSocket upgrader to handle WebSocket connections
var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

var (
	agents     = make(map[string]*websocket.Conn) // Map to store WebSocket connections of connected agents
	guiClients = make(map[string]*websocket.Conn) // Map to store WebSocket connections of connected GUI clients
	mu         sync.Mutex                         // Mutex to synchronize access to the agents and guiClients maps
)

func main() {
	router := gin.Default()

	// Serve the web GUI
	router.Static("/static", "./static")
	router.GET("/", func(c *gin.Context) {
		c.Redirect(http.StatusMovedPermanently, "/static/index.html")
	})

	// WebSocket endpoint for agents
	router.GET("/agent", func(c *gin.Context) {
		conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
		if err != nil {
			fmt.Println("[-] WebSocket upgrade failed for agent:", err)
			return
		}

		defer conn.Close()
		clientID := conn.RemoteAddr().String()
		mu.Lock()
		agents[clientID] = conn
		mu.Unlock()

		fmt.Println("[+] New agent connected:", clientID)

		// Read responses from agents and forward to GUI clients
		for {
			_, message, err := conn.ReadMessage()
			if err != nil {
				fmt.Println("[-] Connection with agent lost:", clientID)
				mu.Lock()
				delete(agents, clientID)
				mu.Unlock()
				break
			}

			fmt.Printf("[Agent %s]: %s\n", clientID, string(message))
			broadcastToGUIClients(fmt.Sprintf("[Agent %s]: %s", clientID, string(message)))
		}
	})

	// WebSocket endpoint for commands
	router.GET("/command", func(c *gin.Context) {
		conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
		if err != nil {
			fmt.Println("[-] Command WebSocket upgrade failed:", err)
			return
		}

		defer conn.Close()
		for {
			_, message, err := conn.ReadMessage()
			if err != nil {
				fmt.Println("[-] Command WebSocket closed.")
				break
			}

			command := string(message)
			fmt.Println("[+] Received command:", command)

			// Broadcast the command to all agents
			mu.Lock()
			for id, agentConn := range agents {
				err := agentConn.WriteMessage(websocket.TextMessage, []byte(command))
				if err != nil {
					fmt.Println("[-] Error sending command to agent:", id)
					delete(agents, id)
				}
			}
			mu.Unlock()
		}
	})

	// WebSocket endpoint for GUI clients
	router.GET("/gui", func(c *gin.Context) {
		conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
		if err != nil {
			fmt.Println("[-] WebSocket upgrade failed for GUI client:", err)
			return
		}

		defer conn.Close()
		clientID := conn.RemoteAddr().String()
		mu.Lock()
		guiClients[clientID] = conn
		mu.Unlock()

		fmt.Println("[+] New GUI client connected:", clientID)

		// Keep the connection alive
		for {
			_, _, err := conn.ReadMessage()
			if err != nil {
				fmt.Println("[-] Connection with GUI client lost:", clientID)
				mu.Lock()
				delete(guiClients, clientID)
				mu.Unlock()
				break
			}
		}
	})

	router.Run(":8080")
}

// Broadcast messages to all GUI clients
func broadcastToGUIClients(message string) {
	mu.Lock()
	defer mu.Unlock()

	for id, guiConn := range guiClients {
		err := guiConn.WriteMessage(websocket.TextMessage, []byte(message))
		if err != nil {
			fmt.Println("[-] Error sending message to GUI client:", id)
			delete(guiClients, id)
		}
	}
}
