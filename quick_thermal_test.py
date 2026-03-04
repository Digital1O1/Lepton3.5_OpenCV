#!/usr/bin/env python3
import cv2
import numpy as np

cap = cv2.VideoCapture(0)  # /dev/video16
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('Y','1','6',' '))

print("Press 'q' to quit")

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    # Normalize for display
    display = cv2.normalize(frame, None, 0, 255, cv2.NORM_MINMAX)
    display = cv2.applyColorMap(display.astype('uint8'), cv2.COLORMAP_JET)
    display = cv2.resize(display, (640, 480))
    
    cv2.imshow('Thermal', display)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
