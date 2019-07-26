"""
**************************************************************************
*                  E-Yantra Robotics Competition
*                  ================================
*  This software is intended to check version compatiability of open source software
*  Theme: Thirsty Crow
*  MODULE: Task 4
*  Filename: Original Configuration.py
*  Date: February 23, 2019
*  e-Yantra - An MHRD project under National Mission on Education using 
*  ICT(NMEICT)
*
*************************************************************************************
*  Team Id : 285
*  Auther list : Sanket Patil, Aashish Shirgave, Yash Agrawal, Yadnyi Deshpande.
*  Functions : main, getCameraMatrix, init_gl, resize, drawGLscene,
*                 draw_background, detect_markers, init_object_texture, overlay
*  Global variables : texture_object,texture_background,camera_matrix,dist_coeff,
*               crow,wpitcher1,wpitcher2,wpitcher3,wpitcher4,pebbledim,pebblefull,
*               arena_config,pebbles_done,pebble,send_flag,string,rawsank,xsize,ysize,flag

"""
####################################################################################

#Imporinting required modules
import numpy as np
import cv2
import cv2.aruco as aruco
import math
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from PIL import Image
import pygame
from objloader import *
import serial
import time

#Declaring global variables
texture_object = None
texture_background = None
camera_matrix = None
dist_coeff = None
cap = cv2.VideoCapture(1)
crow = None
wpitcher = None
wpitcher4 = None
wpitcher3 = None
wpitcher2 = None
wpitcher1 = None
pebbledim= None
pebblefull = None
arena_config = None
pebbles_done = None
pebble = None
send_flag = None
string = None
rawsank = None
xsize , ysize = 0 , 0
flag = 1

INVERSE_MATRIX = np.array([[ 1.0, 1.0, 1.0, 1.0],
                           [-1.0,-1.0,-1.0,-1.0],
                           [-1.0,-1.0,-1.0,-1.0],
                           [ 1.0, 1.0, 1.0, 1.0]])

ser = serial.Serial("COM7", 9600, timeout=0.005)    #Opening serial port of xbee communication

"""
Function Name : getCameraMatrix()
Input: None
Output: camera_matrix, dist_coeff
Purpose: Loads the camera calibration file provided and returns the camera and
         distortion matrix saved in the calibration file.
"""
def getCameraMatrix():
        global camera_matrix, dist_coeff
        #Reading from camera.npz file
        with np.load('Camera_new.npz') as X :
                camera_matrix, dist_coeff, _, _ = [X[i] for i in ('mtx','dist','rvecs','tvecs')]




"""
Function Name : main()
Input: None
Output: None
Purpose: Initialises OpenGL window and callback functions. Then starts the event
         processing loop.
"""        
def main():
        global arena_config
        global pebbles_done
        global pebble
        global string
        
        #Given configuration
        arena_config = {0: ("Water Pitcher", 8, "3-3"), 2:("Pebble",10, "1-1"), 3:("Pebble",2, "2-2"),5:("Pebble",17, "3-3")}
        Robot_start = "START-1"
        
        #Collcting information from given configuration for xbee communication into string
        string = Robot_start[6]
        for i in arena_config :
            if arena_config[i][0] == "Water Pitcher":
                chw = arena_config[i][1] + 96
                positionw = chr(chw)
                oriw = arena_config[i][2][0]
                break;
        
        #setting preference order for pebble pickup 
        pebble = 0
        pebbles_done = np.zeros((10,2))
        pos_1 = [14,5,15,6,11,2,16,7,3,12,1,17] #preferences for start-1
        pos_2 = [10,9,19,8,13,4,18]             #preferences for start-2
        m = 0
        
        #formation of string when start is start-1
        if Robot_start == 'START-1' :
                #first give preferences to positions near start-1 in arena_config
                for i in arena_config :
                        if arena_config[i][0] == "Pebble" :
                                p = arena_config[i][1]
                                for j in pos_1 :
                                        if p == j :
                                                ch = arena_config[i][1] + 96
                                                position = chr(ch)
                                                ori = arena_config[i][2][0]                                        
                                                string += position + ori
                                                string += positionw + oriw
                                                pebbles_done[m][0] = i
                                                pebbles_done[m][1] = 0
                                                m = m + 1 
                #remaining pebbles from arena_config                        
                for i in arena_config :
                        if arena_config[i][0] == "Pebble" :
                                p = arena_config[i][1]
                                for j in pos_2 :
                                        if p == j :
                                                ch = arena_config[i][1] + 96
                                                position = chr(ch)
                                                ori = arena_config[i][2][0]                                        
                                                string += position + ori
                                                string += positionw + oriw
                                                pebbles_done[m][0] = i
                                                pebbles_done[m][1] = 0
                                                m = m + 1
        #formation of string when start is start-2                      
        if Robot_start == 'START-2' :
                #first give preferences to positions near start-2 in arena_config
                for i in arena_config :
                        if arena_config[i][0] == "Pebble" :
                                p = arena_config[i][1]
                                for j in pos_2 :
                                        if p == j :
                                                ch = arena_config[i][1] + 96
                                                position = chr(ch)
                                                ori = arena_config[i][2][0]                                        
                                                string += position + ori
                                                string += positionw + oriw
                                                pebbles_done[m][0] = i
                                                pebbles_done[m][1] = 0
                                                m = m + 1
                #remaining pebbles from arena_config append to string
                for i in arena_config :
                        if arena_config[i][0] == "Pebble" :
                                p = arena_config[i][1]
                                for j in pos_1 :
                                        if p == j :
                                                print(p)
                                                ch = arena_config[i][1] + 96
                                                position = chr(ch)
                                                ori = arena_config[i][2][0]                                        
                                                string += position + ori
                                                string += positionw + oriw
                                                pebbles_done[m][0] = i
                                                pebbles_done[m][1] = 0
                                                m = m + 1

        print(string)
        #send flag for one time tranmission of data to robot
        global send_flag
        send_flag = 0
        
        glutInit()  #initializes GLUT
        getCameraMatrix()   #function call to set camera matrix in global camera_matrix
        glutInitWindowSize(640, 480)    #sets size of screen window in pixels
        glutInitWindowPosition(660, 100)    #sets initial window position on the screen for upper left corner
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE)    # sets the basic display modes
        window_id = glutCreateWindow("OpenGL")  #creates a top-level window labeled with title
        glutDisplayFunc(drawGLScene)    #sets the callback function
        glutIdleFunc(drawGLScene)   # registers the callback for idle time
        init_gl()   #function call to intialize GL screen
        #glutReshapeFunc(resize)
        glutMainLoop()  #start the event loop; never returns

"""
Function Name : init_gl()
Input: None
Output: None
Purpose: Initialises various parameters related to OpenGL scene.
"""  
def init_gl():
        global texture_object, texture_background
        global crow
        global wpitcher1
        global wpitcher2
        global wpitcher3
        global wpitcher4
        global pebblefull
        global wpitchere
        global pebbledim
        
        glClearColor(0.0, 0.0, 0.0, 0.0)    #set the clear color (red, green, blue)
        glClearDepth(1.0)   # clear the depth buffer
        glDepthFunc(GL_LESS)    #Specifies the value used for depth-buffer comparisons
        glEnable(GL_DEPTH_TEST) #Enables OpenGL capabilities
        glShadeModel(GL_SMOOTH)   #Selects flat or smooth shading
        glMatrixMode(GL_MODELVIEW)  #Specifies which matrix is the current matrix
        glEnable(GL_DEPTH_TEST) #Enables  OpenGL capabilities
        glEnable(GL_LIGHTING)   #Enables  OpenGL capabilities
        glEnable(GL_LIGHT0)     #Enables  OpenGL capabilities
        
        texture_object = glGenTextures(1)   #Generates texture names
        crow = OBJ('Crow.obj', swapyz=True) #function call to OBJ 
        wpitcher1 = OBJ('waterpitcher1.obj', swapyz=True)
        wpitcher2 = OBJ('waterpitcher2.obj', swapyz=True)
        wpitcher3 = OBJ('waterpitcher3.obj', swapyz=True)
        wpitcher4 = OBJ('waterpitcher4.obj', swapyz=True)
        pebbledim = OBJ('pebbledim22feb.obj', swapyz=True)
        pebblefull = OBJ('pebblepile22feb.obj', swapyz=True)
        glEnable(GL_TEXTURE_2D) #Enables  OpenGL texture
        texture_background = glGenTextures(1)   #Generates texture names
        texture_object = glGenTextures(1)   #Generates texture names
        
"""
Function Name : resize()
Input: width , height
Output: None
Purpose: Initialises the projection matrix of OpenGL scene
"""
def resize(w,h):
        # w and h are width and height respectively
        w = 480
        h = 640         
        ratio = 1.0* w / h
        glMatrixMode(GL_PROJECTION) #Specifies which matrix is the current matrix
        glViewport(0,0,w,h)     #Sets the viewport
        gluPerspective(45, ratio, 0.1, 100.0)   #defines a perspective projection

"""
Function Name : drawGLScene()
Input: None
Output: None
Purpose: It is the main callback function which is called again and
         again by the event processing loop. In this loop, the webcam frame
         is received and set as background for OpenGL scene. ArUco marker is
         detected in the webcam frame and 3D model is overlayed on the marker
         by calling the overlay() function.
"""
def drawGLScene():
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)  #clears buffers to preset values
        glLoadIdentity()    #Replaces the current matrix with the identity matrix
        ar_list = []
        ret, frame = cap.read()     #reading image in frame
        global pebble
        global send_flag
        global string
        global pebble_done
        if ret == True:
                draw_background(frame)  #function call draw_background
                if send_flag == 0 :
                        #initial communication with robot to confirm robot is ready to reiceive arena configuration    
                        ch = '%'
                        ser.write(str.encode(ch))   #sends ch to xbee
                        time.sleep(0.1) #waits for communication
                        data = ser.read(5)  #read response of 5 bytes from xbee of robot
                        time.sleep(0.1) #waits for communication
                        print(data)

                        if data :
                                #sends string to xbee starting and ending with character * 
                                ch = "*"
                                ser.write(str.encode(ch)) #string character(*) of string
                                time.sleep(0.1)
                                data = ser.read(50)
                                time.sleep(0.1)
                                print(data)
                                for ch in string :
                                    print(ch)
                                    ser.write(str.encode(ch))
                                    time.sleep(0.1)
                                    data = ser.read(50)
                                    time.sleep(0.1)
                                    print(data)
                                ch = "*"
                                ser.write(str.encode(ch)) #ending character(*) of string
                                time.sleep(0.1)
                                data = ser.read(50)
                                time.sleep(0.1)
                                print(data)
                                send_flag = 1   #set send_flag so that communication of string occures only once
                
                ar_list = detect_markers(frame) #call for function detect_markers
                data = ser.read(50)
                time.sleep(0.05)
                k = '#'
                k = str.encode(k)
                if data :
                        print(data)
                if data == k :                 
                        pebble = pebble + 1     #counts pickups + drops in variable pebble
                        if pebble % 2 == 1 :
                                pebbles_done[math.floor(pebble / 2)][1] = 1
                        print(pebble)
                for i in ar_list:
                        #overlay function calls according to aruco ids
                        if i[0] == 0:
                                overlay(frame, ar_list, i[0],"texture.png")
                        if i[0] == 1:
                                overlay(frame, ar_list, i[0],"texture_2.png")
                        if i[0] == 2:
                                overlay(frame, ar_list, i[0],"texture.png")
                        if i[0] == 3:
                                overlay(frame, ar_list, i[0],"texture_2.png")                                
                        if i[0] == 4:
                                overlay(frame, ar_list, i[0],"texture2.png")                        
                        if i[0] == 6:
                                overlay(frame, ar_list, i[0],"texture1.png")
                        if i[0] == 7:
                                overlay(frame, ar_list, i[0],"texture.png")
                        if i[0] == 8:
                                overlay(frame, ar_list, i[0],"texture.png")
                        if i[0] == 9:
                                overlay(frame, ar_list, i[0],"texture.png")
                        if i[0] == 10:
                                overlay(frame, ar_list, i[0],"crow.png")
                
                cv2.imshow('frame', frame)  #display of opencv frame
                cv2.waitKey(1)  #Waits for a pressed key
        glutSwapBuffers()   #swaps the buffers of the current window


######################## Aruco Detection Function ######################
"""
Function Name : detect_markers()
Input: img (numpy array)
Output: aruco list in the form [(aruco_id_1, centre_1, rvec_1, tvec_1),(aruco_id_2,
        centre_2, rvec_2, tvec_2), ()....]
Purpose: This function takes the image in form of a numpy array, camera_matrix and
         distortion matrix as input and detects ArUco markers in the image. For each
         ArUco marker detected in image, paramters such as ID, centre coord, rvec
         and tvec are calculated and stored in a list in a prescribed format. The list
         is returned as output for the function
"""
def detect_markers(img):
        aruco_list = []     #declaration of aruco_list
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)    #convert image to gray
        aruco_dict = cv2.aruco.Dictionary_get(cv2.aruco.DICT_5X5_250)   #aruco dictionary of 5X5
        parameters = cv2.aruco.DetectorParameters_create()  #parameters for detection
        ids = []    #declaration of list of ids
        corners, ids, rejectedImgPoints = cv2.aruco.detectMarkers(gray, aruco_dict,
                    parameters = parameters)    #Basic marker detection
        img = aruco.drawDetectedMarkers(img,corners,ids)    #write ids on respective markers
        rvec,tvec, sanket= cv2.aruco.estimatePoseSingleMarkers(corners,100, camera_matrix, dist_coeff)  #Pose estimation for single markers
        try :
            x = len(ids)
        except :
            x = 0 #exceptional case when no id found
            
        for i in range(x):
            pts = np.float32([[0,0,0]]) #numpy array declaration
            imgpts, _ = cv2.projectPoints(pts, rvec[i], tvec[i], camera_matrix,dist_coeff)  # transform the object point to image point
            centre = tuple(imgpts.ravel()) #declare tuple of centre
            tup = (ids[i][0],centre,rvec[i][0],tvec[i][0])  #add values in tuple
            aruco_list.append(tup)  #append tuple to list
        #return aruco_list 
        return aruco_list 
########################################################################

"""
Function Name : draw_background()
Input: img (numpy array)
Output: None
Purpose: Takes image as input and converts it into an OpenGL texture. That
         OpenGL texture is then set as background of the OpenGL scene
"""
def draw_background(img):
    
    tx_image = cv2.flip(img, 0) #flip frame
    tx_image = Image.fromarray(tx_image)    #image from numpy array 
    xsize = tx_image.size[0]    # X-dimention of image in xsize 
    ysize = tx_image.size[1]    # Y-dimention of image in ysize
    rawsank = tx_image.tobytes('raw', 'BGR', 0, 1)  #image to bytes conversion

    glClearColor ( 0, 0, 0, 0.0 )   #set the clear color (red, green, blue)
    glShadeModel( GL_SMOOTH )   # 	Selects  smooth shading
    #BindTextureEnables the creation of a named texture that is bound to a texture target
    glBindTexture(GL_TEXTURE_2D, texture_object)
    
    #Setting of texture parameters
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT )
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT )
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR )
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR )
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    #specifies a two-dimensional texture image
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, xsize, ysize, 0,
				 GL_RGB, GL_UNSIGNED_BYTE, rawsank)
    glEnable(GL_TEXTURE_2D)
    
    glBegin(GL_QUADS)
    glTexCoord2f( 0, 1 )
    glVertex3f( -1, 1, 0 )
    glTexCoord2f( 0, 0 )
    glVertex3f( -1, -1, 0 )
    glTexCoord2f( 1, 0 )
    glVertex3f( 1, -1, 0 )
    glTexCoord2f( 1, 1 )
    glVertex3f( 1, 1, 0 )
    glEnd()
    #glDisable( GL_TEXTURE_2D )
    return None

"""
Function Name : init_object_texture()
Input: Image file path
Output: None
Purpose: Takes the filepath of a texture file as input and converts it into OpenGL
         texture. The texture is then applied to the next object rendered in the OpenGL
         scene.
"""
def init_object_texture(image_filepath):
        tex = cv2.imread(image_filepath)
        tx_image = cv2.flip(tex, 0)
        tx_image = Image.fromarray(tx_image)
        xsize = tx_image.size[0]
        ysize = tx_image.size[1]
        rawsank = tx_image.tobytes('raw', 'RGB', 0, 1)
        """
        im = Image.open("1.jpg")
        xsize,ysize = im.size[0],im.size[1]
        rawsank = im.tobytes("raw", "RGBX", 0, -1)
        """
        glClearColor ( 0, 0, 0, 0.0 )
        glShadeModel( GL_SMOOTH )
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT )
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT )
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR )
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR )
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, xsize, ysize, 0,
				 GL_RGB, GL_UNSIGNED_BYTE, rawsank)
        glEnable( GL_TEXTURE_2D )   ##Enables  OpenGL capabilities
        return None

"""
Function Name : overlay()
Input: img (numpy array), aruco_list, aruco_id, texture_file (filepath of texture file)
Output: None
Purpose: Receives the ArUco information as input and overlays the 3D Model of a teapot
         on the ArUco marker. That ArUco information is used to
         calculate the rotation matrix and subsequently the view matrix. Then that view matrix
         is loaded as current matrix and the 3D model is rendered.

"""
def overlay(img, ar_list, ar_id, texture_file):
        
        for x in ar_list:
                if ar_id == x[0]:
                        centre, rvecs, tvecs = x[1], x[2], x[3]
        rmtx = cv2.Rodrigues(rvecs)[0]  #Converts a rotation matrix to a rotation vector
        r = 1
        a = centre[0]
        b = centre[1]
        #setting variables p and q using centre to use in view matrix 
        if centre is not None:
            q = (b - (240)) / 240
            p = (a - (320)) / 320
            
        #setting values to view matrix using rotation matrix and variables p ,q formed above
        view_matrix = np.array([[rmtx[0][0],rmtx[0][1],rmtx[0][2],p],
                        [rmtx[1][0],rmtx[1][1],rmtx[1][2],q],
                        [rmtx[2][0],rmtx[2][1],rmtx[2][2],1],
                        [0.0       ,0.0       ,0.0       ,1.0]])
         
        view_matrix = view_matrix * INVERSE_MATRIX
        view_matrix = np.transpose(view_matrix) #transpose of view_matrix
        
        #glutReshapeFunc(resize)
        if ar_id == 10 :
                #BindTexture Enables the creation of a named texture that is bound to a texture target
                glBindTexture(GL_TEXTURE_2D, texture_object)
                init_object_texture(texture_file) #binds black texture to ids except id 0(water picture)
        glPushMatrix()  #Push the current matrix stack
        glLoadMatrixd(view_matrix)  #replace the current matrix with an arbitrary matrix
        
        #glCallList function ( called below )executes  display list given as parameter
        
        #display of water pitcher on aruco id - 0
        if ar_id == 0 :
                #code for animation of water pitcher with increasing water level according to pebble count
                if math.floor(pebble / 2)  == 0 :
                        glCallList(wpitcher1.gl_list)
                elif math.floor(pebble / 2) == 1 :
                        glCallList(wpitcher2.gl_list)
                elif math.floor(pebble /2) == 2 :
                        glCallList(wpitcher3.gl_list)
                elif math.floor(pebble / 2) == 3 :
                        glCallList(wpitcher4.gl_list)
        else :
                for i in pebbles_done :
                        if i[0] == ar_id :
                                #code for animation of pebble before and after drop
                                if i[1] == 0 :
                                        glCallList(pebblefull.gl_list)
                                else :
                                        glCallList(pebbledim.gl_list)
        if ar_id == 10 :
                #display of crow on aruco id - 10
                glCallList(crow.gl_list)

        glPopMatrix()   #Pop the current matrix stack

########################################################################

if __name__ == "__main__":
        main()

        
