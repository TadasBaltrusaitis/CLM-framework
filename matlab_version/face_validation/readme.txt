To create the training data run:
Create_data_68_large.m or Create_data_66_large.m for 68 and 66 point PDM versions accordingly.

The data generation code requires you to have the patch expert training data (Multi-PIE and in-the-wild data, not included) for positive examples, and inriaperson dataset for negative samples (not included as well). 

To train Convolutional Neural Network based face landmark validation model (used by the model now) use:
Train_face_checker_66_cnn.m and Train_face_checker_68_cnn.m

This will produce face_check_cnn_*.mat and face_check_cnn_*.txt files that can be used in C++ and matlab versions of CLM framework for face checking.

This will also produces tris*.txt files that can be used in the C++ version of the CLM_framework.

The code uses piece-wise affine warping to a neutral shape with an SVR regressor for error estimation (see http://www.cl.cam.ac.uk/~tb346/ThesisFinal.pdf Section 4.6.2 for more details)