### **Project Title: Smart Medibox using ESP32 on Wokwi Platform**

**Project Overview**:  
This project involves the design and simulation of a Smart Medibox using the Wokwi platform and ESP32 microcontroller. The primary aim is to assist users in managing their medication schedule effectively while monitoring environmental conditions such as temperature and humidity, which may affect medication storage. It incorporates a variety of sensors and actuators to enhance its functionality and is integrated with a Node-RED dashboard, utilizing MQTT protocol for communication.

---

### **Key Components**:

1. **Wokwi Platform Simulation**:  
   The project is simulated on the Wokwi platform, a powerful tool for building and testing ESP32-based projects virtually before physical deployment.

2. **Node-RED Dashboard with MQTT Client**:  
   A dashboard on Node-RED is utilized to provide a real-time interface with the Medibox. The MQTT protocol is used to publish and subscribe to data streams between the ESP32 and the dashboard. The interface allows the user to view real-time environmental data from the Medibox and monitor medication alerts.

3. **Medication Reminder System**:  
   The smart Medibox reminds the user to take their medication at preset times by sounding an alarm. The user can set up to three different alarms, ensuring they never miss a dose. These alarms are configured through a navigable menu.

4. **Environmental Monitoring (DHT11 Sensor)**:  
   The Medibox is equipped with a DHT11 sensor to monitor temperature and humidity. If either value exceeds preset thresholds, the system will trigger an alarm, alerting the user to potential environmental risks that could compromise the medication's quality.

5. **NTP Client for Time Display**:  
   The system features an NTP (Network Time Protocol) client, ensuring that the time displayed on the device is accurate. This is crucial for managing medication times and maintaining proper functionality.

---

![image](https://github.com/user-attachments/assets/952e3b9e-a920-4efa-80ee-3aa10fe64751)


### **Hardware Components**:

- **ESP32 Microcontroller**: Acts as the brain of the system, handling sensor data, user input, and communication with the Node-RED dashboard.
- **Push Buttons**: Four push buttons are used to navigate through the menu system, enabling the user to set alarms and configure other system options.
- **LCD Display**: Provides a user-friendly interface to display time, environmental readings, and alarm settings.
- **DHT11 Sensor**: Measures temperature and humidity inside the Medibox to ensure that medication is stored under appropriate conditions.
- **Buzzer**: Sounds an alarm when medication is due or when environmental parameters exceed safe levels.
- **Servo Motor**: Adjusts the angle of the Medibox lid based on the environmental conditions inside the box. If temperature or humidity crosses a threshold, the angle of the servo motor changes automatically to regulate the environment.
  
---

### **Menu and Alarms**:

- The Medibox features a navigable menu system, allowing the user to set time and alarms using the four push buttons. The user can set three separate alarms for different medication times.
- The Node-RED dashboard also displays real-time data from the Medibox, showing the current status of the medication box and any alarms that are set.
  
---

### **Automation of Servo Motor**:  
The angle of the servo motor is controlled automatically based on the humidity and temperature readings from the DHT11 sensor. If the environmental conditions deviate from preset safe levels, the system adjusts the motor to open or close the Medibox slightly, helping to maintain optimal storage conditions.

---

### **Conclusion**:  
This Smart Medibox project effectively combines embedded systems, IoT technology, and environmental monitoring to assist users in managing their medication schedules and ensuring that medications are stored in optimal conditions. By integrating a Node-RED dashboard and using MQTT protocol for real-time data communication, the project provides a robust solution for medication management and monitoring.

