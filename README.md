# Advanced Alarm Clock System

The Advanced Alarm Clock System is an alarm clock project that combines the power of an Arduino board and Python to create a final project. This system leverages Python for core operations, integrating multiple libraries to handle serial communication, asynchronous programming, and real-time weather data fetching.

## Features

- **Dynamic Weather Updates**: The system periodically fetches weather updates for a specified location, displaying this information on the Arduino display to keep you informed of local weather conditions.
- **Proximity Interaction**: If a user is close to the clock, the system prompts to set a timer, enhancing interactive functionality.
- **Customizable Timer**: Users can set the timer duration through motion sensor interaction, moving their hand back and forth to increase or decrease the timer amount.
- **Advanced Alarm Management**: Users can cancel the alarm or initiate a snooze function, which resets the timer for half of the original time. If the alarm triggers in a dark room, the system will activate a light flickering feature and tone using the speaker.
- **Asynchronous Programming**: This system efficiently handles I/O-bound operations using Python's asyncio library.
- **Serial Communication**: The Python program communicates with the Arduino through a serial connection, providing real-time updates and data receipt.

## Installation & Setup

Follow the steps below to set up the Advanced Alarm Clock System:

1. **Clone the Repository**

   ```
   git clone https://github.com/TacoWound/FinalProject444.git
   ```

2. **Setup Python Environment**
   Install Python 3.7 or higher and use a virtual environment.

   ```
   python3 -m venv venv
   source venv/bin/activate
   ```

3. **Install Required Python Libraries**

   ```
   pip install -r requirements.txt
   ```

4. **Setup Serial Connection**
   Update the port name in **main.py** to match your system's configuration (e.g., 'COM3' for Windows or '/dev/ttyACM0' for Unix-like systems).
5. **Update City**
   Replace "Winnipeg" with your city name in the **get_weather()** function call within the **main_loop()** function.
6. **Setup Arduino**
   Upload the Arduino code to your Arduino board using the Arduino IDE.
7. **Hardware Setup**
   Connect the required sensors as per the hardware setup guide.
8. **Run the Python Script**

   ```
   python clock_helper.py
   ```

## How to Use

- Stand near the clock or press the button to initiate the timer setup.
- Press the button to confirm timer setup, and use hand movements back and forth to adjust the timer.
- Press the button again to start the timer.
- Press the button and swipe to cancel the alarm.
- If the alarm goes off and the room is dark, lights will flicker and tone will flicker.
- To start snooze, swipe when the timer goes off.

## Project Iterations

The project was developed over four iterations. Each phase focused on introducing new features and improving the overall user experience.

1. **Iteration 1**: Integrated weather API to display real-time weather and clock.
2. **Iteration 2**: Introduced proximity detection to prompt the user to set a timer.
3. **Iteration 3**: Added customizable timer and advanced alarm management features.
4. **Iteration 4**: Incorporated light control for alarm alerts in dark environments and added snooze functionality.

## How to Contribute

Contributions are greatly appreciated and help make the open-source community an amazing place to learn, inspire, and create.

1. Fork the project.
2. Create your feature branch (`git checkout -b feature/NewFeature`).
3. Commit your changes (`git commit -m 'Add some NewFeature'`).
4. Push to the branch (`git push origin feature/NewFeature`).
5. Open a pull request.

## Troubleshooting

Please open a new issue if you encounter problems while setting up or using the system.


Project Link: <https://github.com/TacoWound/FinalProject444>
