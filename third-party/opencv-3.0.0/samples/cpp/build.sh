#!/bin/bash
g++ facedetect.cpp -o facedetect `pkg-config --cflags --libs opencv` 
