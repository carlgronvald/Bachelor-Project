# -*- coding: utf-8 -*-
"""
Created on Wed Sep 23 15:46:41 2020

@author: Carl
"""

import pandas as pd
import os
import cv2
import numpy as np

A = cv2.UMat(np.array([[3838.27,0,2808],[0,3837.22,1872],[0,0,1]]))

image_file_path = "gerrard-hall\\gerrard-hall\\dense\\0\\sparse\\images.txt"
image_file_output_path = "gerrard-hall\\gerrard-hall\\dense\\0\\view.txt"
image_folder = "gerrard-hall\\gerrard-hall\\dense\\0\\images2\\"

rename_images = True

def renameImage(imageName, ID):
    os.rename(image_folder + imageName.strip(), image_folder + "view" + str(ID).zfill(3) + ".jpg")

with open(image_file_path) as finput:
    with open(image_file_output_path, 'w') as foutput:
        foutput.write("ID	QW	QX	QY	QZ	TX	TY	TZ	PID\n")
        line = ""
        for i in range(0,5):
            line = finput.readline();
        
        parts = []
        while line:
            
            parts = line.split(" ")
            if rename_images:
                renameImage(parts[-1], int(parts[0]))
            
            newLine = ""
            for part in parts[0:-2]:
                newLine += part + " "
            newLine += parts[0] + "\n"
            
            foutput.write(newLine);
            line = finput.readline();
            line = finput.readline();