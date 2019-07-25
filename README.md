# E-Yantra-Project
This is repository for the project done by our team for E-Yantra Robotics Competition 2018

<h3>About competition</h3>
<p>e-Yantra Robotics Competition (eYRC) is a unique annual competition for undergraduate students in science and colleges. Selected teams are given a robotic kit complete with accessories and video tutorials to help them learn basic concepts in embedded systems and microcontroller programming. Abstracts of real world problems assigned as "themes" are then implemented by the teams using the robotic kits. The theme assigned to the team is <b>"Thirsty Crow"</b>. </p>


<h3>Thirsty Crow</h3>
<p>The story of Crow and Water Pitcher is one of Aesop’s Fables which everybody has heard in their childhoods. This theme aims to bring this age-old able to life using Augmented Reality with it’s theme - Thirsty Crow. A robot called Crow is deployed in a field where it has to pick up pebbles from different locations in an Arena and drop them at a specific location labeled as Water Pitcher. At the same time, on a virtual screen the story comes to life with the help of AR and special markers. 3D models of Crow, Pebbles and Pitcher (created by teams) are projected on an image of the arena.
 
<p>The aim of the theme is to complete the task in the shortest time possible. The team which deposits all the pebbles in the water pitcher and projects all 3D models correctly will be declared the WINNER.</p>
<h3>Technology Envolved</h3>
<ul>
  <li>Image Processing</li>
  <li>Graphics Processing using OpenGL</li>
  <li>Microcontroller Programming</li>
  <li>Blender Model Creation</li>
</ul>

<h3>Project Description</p>
<p>The arena for the project is as follows :</p> 
<p align="center">
 <img src = "images/arena_d.jpg">
</p>

<h3>Flow of the project</h3>
<ol>
  <li>  Python script is started. The image of arena is projected on screen with 3D models being projected on top of AR_Objects.</li>
  <li>Robot is placed (switched OFF) on either START-1 or START-2 according to configuration. robot is switched ON.</li>
  <li>Robot traverses to each of the Pebble AR_Object locations and picks up the magnetic pebble from under the AR_Object.</li>
  <li>During pebble pickup, augmented reality part shows Pebble Pickup animation.</li>
  <li>After each pebble pickup, robot traverses to Water Pitcher AR_Object to deposit the magnetic pebble in the    deposition/pickup zone under the AR_Object.</li>
  <li>During pebble drop, augmented reality part should show Pebble Drop animation.</li>
  <li>After all magnetic pebbles have been deposited under the Water Pitcher AR_Object, robot will sound the buzzer for 5 seconds to end the task.</li>
 </ol>
  
