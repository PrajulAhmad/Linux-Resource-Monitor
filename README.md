# Linux System Resource Monitor 🐧

A lightweight, command-line system monitoring tool written in C. It provides real-time insights into system performance, similar to standard utilities like `top` or `htop`.

![Project Status](https://img.shields.io/badge/status-active-brightgreen)
![Language](https://img.shields.io/badge/language-C-blue)
![Platform](https://img.shields.io/badge/platform-Linux-orange)

## 📖 Overview

This project is an engineering exploration into how Linux exposes system data. Instead of using high-level libraries, this tool manually parses the Linux `/proc` filesystem to calculate CPU usage, memory consumption, and process states in real-time.

It features a custom-built text-based interface (TUI) that uses ANSI escape codes for formatting and "Raw Mode" for responsive user input without blocking.

## ✨ Features

* **Real-Time Dashboard**: Visual bars for global System CPU and RAM usage.
* **Process Monitoring**: Lists active processes with PID, Name, Memory (MB), and accurate CPU %.
* **Responsive UI**: Custom non-blocking input handling (via `select()`) allows instant interaction.
* **Efficient Refresh**: "Smart Wait" logic prevents high CPU consumption by the monitor itself (0% overhead).
* **Modular Architecture**: Clean separation of concerns (Data Collection vs. Analysis vs. Display).

## 🛠️ Technical Implementation

* **Language**: C (C99 Standard)
* **Data Source**: `/proc` filesystem (parsing `/proc/stat`, `/proc/meminfo`, `/proc/[pid]/stat`).
* **CPU Calculation**: Implements differential calculation: $\Delta \text{Active} / \Delta \text{Total} \times 100$.
* **Terminal Control**: Uses `<termios.h>` to switch terminal into Raw Mode and Alternate Screen Buffer.

## 🚀 Getting Started

### Prerequisites
* A Linux operating system (Ubuntu, Fedora, Arch, etc.)
* GCC Compiler (`sudo apt install gcc` or `sudo dnf install gcc`)
* Make (`sudo apt install make`)

### Installation & Run

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/yourusername/linux-resource-monitor.git](https://github.com/yourusername/linux-resource-monitor.git)
    cd linux-resource-monitor
    ```

2.  **Build the project:**
    ```bash
    make
    ```

3.  **Run the monitor:**
    ```bash
    ./monitor
    ```

4.  **Quit:**
    Press `q` or `Ctrl+C` to exit safely.

## 📂 Project Structure

* **`main.c`**: Entry point. Handles the UI loop, terminal settings, and user input.
* **`analyzer.c`**: Logic core. Calculates CPU percentages and manages process history.
* **`reader.c`**: Data fetcher. Parses raw text files from `/proc`.
* **`monitor.h`**: Header file defining data structures (`ProcessInfo`, `GlobalStats`) and prototypes.
* **`Makefile`**: build script.

## 📄 License
This project is open-source and available under the MIT License.
