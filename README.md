# 🚀 5G Virtual Network Security Analysis

## 👥 Team Members
- Nilay Malaviya (202151083) 
- Vedant Patel (202151112) 
- Raj Kashyap (202151128) 
- Vedant Shah (202151143) 
- Dev Parmar (202152327) 

---

## 📖 Project Overview
This project focuses on **analyzing the security vulnerabilities in 5G networks** using a simulated 5G virtual environment. Through **eavesdropping**, **DoS attacks**, and **man-in-the-middle (MITM) attacks**, the project evaluates potential threats and their impacts on network performance and security.

The project workflow involves setting up a **5G virtual network**, capturing packets for analysis, and simulating security threats in a controlled environment to understand their behaviors and mitigation strategies.

### 🌟 Key Features
1. **Setting Up a 5G Virtual Network**  
2. **Packet Capture and Eavesdropping**  
3. **DoS Attack Simulation**  
4. **MITM Attack Analysis**

---

## 🛠 Installation and Setup

### Step 1: Clone the Repository
Clone this repository to your local machine using:
```bash
git clone https://github.com/your-repository/5G-Virtual-Network-Security.git  
cd 5G-Virtual-Network-Security  
```

### Step 2: Install Dependencies
Ensure you have Python installed. Install the required dependencies using:

```bash
pip install -r requirements.txt
```

### Step 3: Set Up NS3 and Wireshark
NS3 Installation: Follow the official NS3 installation guide to set up the simulation environment.
Wireshark Installation: Install Wireshark for packet capture:

```bash
sudo apt update  
sudo apt install wireshark
```

---

### 🔍 Methodology

### 1. Setting Up a 5G Virtual Network
-Used NS3-mmWave module to simulate a 5G network.
-Configured UEs (User Equipment) and eNodeBs (Base Stations) with realistic parameters.
-Enabled packet capture (PCAP) to record network traffic for analysis.

### 2. Eavesdropping on 5G Traffic
-Captured packets using Wireshark during the simulation.
-Identified traffic patterns and potential security leaks from UDP/TCP traffic logs.
-Analyzed headers and payloads to determine the sensitivity of intercepted data.

### 3. DoS Attack Simulation
-Simulated a DoS attack to flood the network with excessive traffic.
-Monitored the impact on latency, throughput, and resource availability.
-Evaluated the system's resilience against high traffic loads and identified potential bottlenecks.

### 4. MITM Attack Analysis
-Set up an MITM attack by intercepting and modifying packets in transit.
-Used custom scripts to replicate unauthorized access to sensitive communications.
-Evaluated how encryption and authentication mechanisms could mitigate such attacks.

### 📊 Results and Insights

### ✅ Packet Analysis (Eavesdropping)
-Successfully intercepted and analyzed UDP/TCP packets.
-Revealed traffic patterns and potential vulnerabilities in unencrypted data flows.

### ✅ DoS Attack Findings
-Significant increase in latency and packet loss during simulated DoS attacks.
-Identified specific components of the network more vulnerable to resource exhaustion.

### ✅ MITM Attack Findings
-Demonstrated the risks of unencrypted or poorly configured communication protocols.
-Highlighted the importance of encryption and secure authentication mechanisms in preventing MITM attacks.

### 🛠 Technologies and Tools Used
-NS3: 5G network simulation.
-Wireshark: Packet capture and traffic analysis.
-Python: For scripting and analysis.
-Mininet: SDN simulation and attack testing.
-OpenSSL: For simulating encryption in network traffic.

### 🔒 Security Recommendations
1. Implement end-to-end encryption for all sensitive communications.
2. Use rate-limiting mechanisms to prevent DoS attacks.
3. Deploy intrusion detection systems to monitor abnormal traffic patterns.
4. Ensure secure key exchange protocols to mitigate MITM attacks.

### 🌐 References
-NS3 Official Documentation
-Wireshark User Guide
-Research Paper: 5G Network Security and Traffic Analysis Using Simulations
