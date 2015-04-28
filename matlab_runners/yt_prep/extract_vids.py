__author__ = 'Tadas'

import cv2
import numpy as np
import glob

# where to find the results.mat:
yt_dir = r"C:\Users\Tadas\Dropbox\AAM\test data\ytceleb_annotations_CVPR2014"


vids = ["0035_02_003_adam_sandler",
        "0042_02_010_adam_sandler",
        "0292_02_002_angelina_jolie",
        "0293_02_003_angelina_jolie",
        "0294_02_004_angelina_jolie",
        "0502_01_005_bruce_willis",
        "0504_01_007_bruce_willis",
        "1198_01_012_julia_roberts",
        "1786_02_006_sylvester_stallone"]

for vid in vids:

    img_dir = yt_dir + "/" + vid + "/"
    yt_imgs = glob.glob(img_dir + '*.png')
    fourcc = cv2.cv.CV_FOURCC('D','I','V','X')
    #fourcc = cv2.VideoWriter_fourcc(*'XVID')
    out = cv2.VideoWriter(yt_dir + "/" + vid + ".avi", -1, 30.0, (320,240))

    for img in yt_imgs:
        img = cv2.imread(img)
        cv2.imshow("test", img)

        cv2.waitKey(10)
        out.write(img)

    out.release()