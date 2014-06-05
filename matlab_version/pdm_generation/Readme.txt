Creating the Point Distribution Model.

To create a model from wild data use the Matlab script:

./Wild_data_pdm/Create_pdm_wild.m (This might take up to a couple of hours, depending on the machine used, and if you compiled computeH - see readme in nrsfm-em folder)

You need the training data which can be acquired from (http://ibug.doc.ic.ac.uk/resources/facial-point-annotations/), to run the script from scratch. Alternatively the data is collected in 'wild_68_pts.mat', so you can skip the Collect_wild_annotations step.

The script will produce "./Wild_data_pdm/pdm_68_aligned_wild.mat" and "./Wild_data_pdm/pdm_68_aligned_wild.txt" which can be used for landmark detection.

To visualise the results use:
visualise_PDMs.m

The PDM triangulation was created using the Delaunay triangulation algorithm, with manual hole cutting for eyes and mouth. Same can be done on any other annotated face dataset.

The pdm used in "in-the-wild" experiments is already included as:
./Wild_data_pdm/pdm_68_aligned_wild
./Wild_data_pdm/pdm_68_aligned_wild.txt

The same model should be generated using the Matlab script as well (overwriting the data).

We use the non-rigid structure from motion approach by Lorenzo Torresani, Aaron Hertzmann, Chris Bregler, "Learning Non-Rigid 3D Shape from 2D Motion", NIPS 16, 2003
http://cs.stanford.edu/~ltorresa/projects/learning-nr-shape/
Please cite their work and ours if you use this code.