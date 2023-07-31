"""
Description: This program will get the current time and weather and send it to the Arduino
Author: Ishraq Md Nazrul  
Student ID: 3613766
Course: COMP 444 
Assignment: Final Project
Date: July 31, 2023
"""

import time
import signal
import sys
import serial
import python_weather
import asyncio
import os

ser = serial.Serial('COM3', 9600)  # default port in PC
current_weather = "NA"

# This function will handle the exit signal (Ctrl+C)


def signal_handler(sig, frame):
    print('Exiting...')
    sys.exit(0)

# This function will get the current weather for the city specified


async def get_weather(city):
    try:
        # async with python_weather to get the weather data
        async with python_weather.Client(unit=python_weather.METRIC) as client:
            print("Getting weather data...")
            weather = await client.get(city)
            print("Got weather data.")
            return f"{weather.current.temperature}C"
        # Exception handling
    except Exception as e:
        print(f"Error getting weather data: {e}")
        return "Error"

# This function will be the main loop of the program


async def main_loop():
    global current_weather          # global variable to store the current weather
    weather_timer = time.time()    # timer to keep track of when to fetch weather

    # Fetch weather at the start
    current_weather = await get_weather("Winnipeg")  # get the current weather

    # Main loop
    while True:

        # if it's been 30 minutes since the last weather fetch to avoid spamming the API
        if time.time() - weather_timer > 1800:  # 1800 seconds = 30 minutes
            print("Fetching weather...")

            # get the current weather
            current_weather = await get_weather("Winnipeg")
            weather_timer = time.time()  # reset the weather timer

        # get the current time and format it
        current_time = time.strftime("%H:%M:%S")  # get the current time
        city = "Winnipeg"  # replace with your city
        data_to_send = current_time + "|" + city + ", " + current_weather

        print(f"Sending data: {data_to_send}")
        # send it over the serial port
        ser.write((data_to_send + '\n').encode())

        while ser.in_waiting:  # if there's data waiting to be read
            # read a line, decode it to ASCII, remove trailing newlines
            arduino_data = ser.readline().decode().strip()
            print(f"Received data from Arduino: {arduino_data}")

        await asyncio.sleep(5)  # wait for 5 seconds

# This is the main entry point for the program
if __name__ == '__main__':
    # Register the signal handler to handle SIGINT (Ctrl+C) otherwise the program can potentially hang
    signal.signal(signal.SIGINT, signal_handler)
    # Windows requires the following line to run asyncio
    if os.name == 'nt':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    print("Starting main loop...")
    asyncio.run(main_loop())
