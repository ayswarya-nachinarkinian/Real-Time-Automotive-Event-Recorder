# Real-Time Automotive Event Recorder

## Project Overview

The **Real-Time Automotive Event Recorder** is a safety-focused embedded system developed using **QNX Neutrino RTOS and Raspberry Pi**. The primary goal of the project is to continuously monitor vehicle conditions using multiple sensors and detect critical events such as **vehicle crashes, rollovers,airbag and fire incidents** in real time.

Unlike a conventional event recorder that only stores information after an accident, this system continuously observes sensor data and identifies abnormal conditions as they occur. When a critical event is detected, the system captures the relevant sensor information along with timestamps and stores it for later analysis and accident reconstruction.

## Problem Statement

During an automotive accident, important information about the moments immediately before and during the incident can be lost. Understanding factors such as sudden acceleration, impact, abnormal rotation, or fire conditions is important for accident investigation and vehicle safety analysis.

The project addresses this problem by creating a **real-time event recording system** capable of detecting and preserving critical sensor information when an abnormal vehicle condition occurs.

## Sensors Used

The system uses multiple sensors to monitor different safety conditions:

* **IMU Sensor:** Measures acceleration and rotational motion of the vehicle. This information is used for detecting sudden impacts and abnormal rotational movement associated with rollover conditions.
* **Temperature Sensor:** Detects the presence of a fire/flame condition due to the change in temperature and generates a corresponding safety event.

Using multiple sensors allows the system to monitor different types of emergency conditions rather than relying on a single measurement.

## Event Detection

The system continuously reads sensor values and compares them against predefined conditions and thresholds.

For a **crash**, sudden abnormal acceleration and impact sensor activation are analyzed to identify a collision.

For a **rollover**, IMU acceleration and gyroscope measurements are analyzed to identify abnormal rotational movement and vehicle orientation.

For a **airbag**, we used switches for now to indicate the crash condition.

For a **fire event**, the fire sensor is continuously monitored and an event is generated when the sensor indicates a fire condition.

Once an event is detected, the system identifies the event type and records the associated sensor information and timestamp.

## Real-Time Processing with QNX

The project uses **QNX Neutrino RTOS** because the event recorder requires predictable and responsive processing of safety-related sensor data.

The application is divided into independent tasks for sensor acquisition, event analysis, buffering, storage, and timing monitoring. Critical processing is given higher scheduling priority so that sensor events can be handled promptly.

QNX's microkernel-based design also provides process isolation, allowing individual components to operate independently and reducing the possibility of a failure in one component affecting the complete event recorder.

## Data Recording

When a critical event occurs, the system records relevant sensor readings together with timing information. The recorded information can subsequently be used to understand the sequence of conditions surrounding the incident.

This enables the system to function as a **digital witness for the vehicle**, preserving important information that can support post-incident investigation and analysis.

## Key Features

* Real-time monitoring of automotive safety conditions
* Crash and impact detection
* Rollover detection using IMU data
* Fire detection
* Timestamped event recording
* Sensor data buffering
* Real-time event analysis
* QNX-based deterministic processing
* Process and thread isolation
* Designed for low-latency event detection

## Technology Used

**Hardware**

* Raspberry Pi 4
* IMU sensor
* Temperature sensor
* Storage

**Software**

* QNX Neutrino RTOS
* QNX Momentics IDE
* C programming
* POSIX/QNX real-time APIs

## Future Scope

The prototype can be further developed toward automotive-grade deployment by using automotive-qualified sensors, rugged storage, improved power protection, redundant sensing, hardware watchdog mechanisms, and more advanced event-analysis algorithms.

The long-term vision is to develop a reliable **real-time automotive safety recorder** capable of capturing critical vehicle events with high timing accuracy and providing detailed information for accident reconstruction and safety improvement.
