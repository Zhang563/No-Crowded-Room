# Project No Crowded Room
> Created A Devide Keeps Tracking How Many People Have Entered/Exited Room to Prevent Overcrowding 

To prevent overcrowding in an indoor area, it is a good practice to keep an indoor place under the maximum occupancy levels. A device is developed to count people as they enter or exit the room. In addition to keeping the room within the designed capacity, the data can also be used to analyze space utilization for better classroom assignment methods or attendance tracking. This provides a wide range of use cases for this project. 


## Arudino Function Description
Arduino is driving a 16x16 LED matrix using FAST LED. A class structure is used to prove custom animation logic. Instead of displaying the number on the LED matrix, more individual LEDs light up as the room approaches the maximum capacity. As LED is added to the matrix, it will pick a random column, starting from the top and dropping to the bottom. It is trying to achieve a "raindrop" effect, and the matrix is the "bucket" as it collects the LED at the bottom.
Using animation instead of displaying a number on the matrix provides an intuitive way for the average person to have a sense of how full the room is. Instead of knowing "there is 10 person in the room", but know "the room is half filled to its designed capacity". There is an auxiliary LCD that displays the absolute count for room administrators, or "stats for the nerds". 
This project was achieved by using two sensors. The sequence of the sensor being triggered determines whether the person is entering or exiting. The data will be passed to the LCD screen and LED matrix.

## Project Circuits
<img width="970" alt="image" src="https://github.com/Zhang563/No-Crowded-Room/assets/60804083/841906d4-34cc-4468-9d29-ebb95a985083">


## Project Housing
A housing was designed using Onshape and laser-cutted on acrylic for assembly. This housing design allows direct mounting onto door frames.
![Assembly Drawing ](https://github.com/Zhang563/No-Crowded-Room/assets/60804083/e5383037-7aed-4070-a210-393a6e41237a)
<img width="1168" alt="laser cutting parts" src="https://github.com/Zhang563/No-Crowded-Room/assets/60804083/7dfb2fb7-595b-475a-a8ce-9d328269a554">

## Highlight of this Project
Interrupt-driven code design. This allows faster response and it is more efficient than polling. It somewhat addresses that a group of people enters the room as it can react to changes fast.

## Limitations
It is designed for single-door rooms, i.e. classrooms as it will only account for events at the door. If the door is wide enough that multiple people can enter and exit at the same instance, this will be less likely to account for this scenario. Future improvements like bringing interconnectivity can bring support to multi-door rooms. 

## Gallery

|![mounted](https://github.com/Zhang563/No-Crowded-Room/assets/60804083/19c7023b-f6ca-498b-b201-f3c1d54b982c)|
|:-:|
|Device Mounted on Door Frame|
|![animation](https://github.com/Zhang563/No-Crowded-Room/assets/60804083/67c21f71-8ac5-4d1b-b39c-b543264b5f42)|
|Demonstration of LED Matrix Animation|
|![handwave](https://github.com/Zhang563/No-Crowded-Room/assets/60804083/ade30299-f346-4035-9f4a-0023c2c66679)|
|Demonstration of Sensing Entering vs. Exiting|

