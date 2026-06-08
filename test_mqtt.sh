#!/bin/bash
# Smart Dustbin MQTT Testing Script
# This script helps test the MQTT communication with the Smart Dustbin

# Configuration
BROKER="turin.iotserver.uz"
PORT="2890"
USERNAME="userTTPU"
PASSWORD="mqttpass"
SUBSCRIBE_TOPIC="dustbin/status"
COMMAND_TOPIC="dustbin/control"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print usage
print_usage() {
    echo "╔══════════════════════════════════════════════════════╗"
    echo "║      Smart Dustbin - MQTT Test Script               ║"
    echo "╚══════════════════════════════════════════════════════╝"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  open      - Send OPEN command to dustbin"
    echo "  close     - Send CLOSE command to dustbin"
    echo "  monitor   - Monitor status messages from dustbin"
    echo "  broker    - Test broker connectivity"
    echo "  help      - Show this help message"
    echo ""
}

# Check if mosquitto-clients is installed
check_mosquitto() {
    if ! command -v mosquitto_pub &> /dev/null; then
        echo -e "${RED}✗ Error: mosquitto-clients is not installed${NC}"
        echo ""
        echo "Install it with:"
        echo "  Ubuntu/Debian: sudo apt-get install mosquitto-clients"
        echo "  macOS:         brew install mosquitto"
        echo "  Others:        Visit https://mosquitto.org/"
        exit 1
    fi
}

# Test broker connectivity
test_broker() {
    echo -e "${BLUE}Testing MQTT Broker connectivity...${NC}"
    echo "Broker: $BROKER:$PORT"
    echo "Username: $USERNAME"
    echo ""
    
    if timeout 5 mosquitto_sub -h "$BROKER" -p "$PORT" \
        -u "$USERNAME" -P "$PASSWORD" \
        -t "$SUBSCRIBE_TOPIC" \
        -W 1 &> /dev/null; then
        echo -e "${GREEN}✓ Broker is reachable!${NC}"
        return 0
    else
        echo -e "${RED}✗ Cannot reach broker${NC}"
        echo "  - Check your internet connection"
        echo "  - Verify broker address: $BROKER"
        echo "  - Verify broker port: $PORT"
        return 1
    fi
}

# Send OPEN command
send_open() {
    echo -e "${YELLOW}Sending OPEN command...${NC}"
    echo "Topic: $COMMAND_TOPIC"
    echo "Payload: OPEN"
    echo ""
    
    mosquitto_pub -h "$BROKER" -p "$PORT" \
        -u "$USERNAME" -P "$PASSWORD" \
        -t "$COMMAND_TOPIC" \
        -m "OPEN"
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Command sent successfully${NC}"
        echo ""
        echo "Expected sequence:"
        echo "  1. Dustbin lid opens (servo moves 0° → 90°)"
        echo "  2. Lid holds open for ~3 seconds"
        echo "  3. Lid auto-closes (servo moves 90° → 0°)"
        echo "  4. Status messages published:"
        echo "     - OPENED"
        echo "     - CLOSED"
    else
        echo -e "${RED}✗ Failed to send command${NC}"
    fi
    echo ""
}

# Send CLOSE command
send_close() {
    echo -e "${YELLOW}Sending CLOSE command...${NC}"
    echo "Topic: $COMMAND_TOPIC"
    echo "Payload: CLOSE"
    echo ""
    
    mosquitto_pub -h "$BROKER" -p "$PORT" \
        -u "$USERNAME" -P "$PASSWORD" \
        -t "$COMMAND_TOPIC" \
        -m "CLOSE"
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Command sent successfully${NC}"
        echo ""
        echo "Expected response:"
        echo "  1. Dustbin lid closes (servo moves to 0°)"
        echo "  2. Status message: CLOSED"
    else
        echo -e "${RED}✗ Failed to send command${NC}"
    fi
    echo ""
}

# Monitor status messages
monitor_status() {
    echo -e "${BLUE}Monitoring status messages from dustbin...${NC}"
    echo "Topic: $SUBSCRIBE_TOPIC"
    echo "Broker: $BROKER:$PORT"
    echo ""
    echo -e "${GREEN}Listening for messages (Ctrl+C to exit)...${NC}"
    echo ""
    
    mosquitto_sub -h "$BROKER" -p "$PORT" \
        -u "$USERNAME" -P "$PASSWORD" \
        -t "$SUBSCRIBE_TOPIC" \
        -v
}

# Main script logic
check_mosquitto

case "$1" in
    open)
        send_open
        ;;
    close)
        send_close
        ;;
    monitor)
        monitor_status
        ;;
    broker)
        test_broker
        ;;
    help)
        print_usage
        ;;
    *)
        print_usage
        ;;
esac
