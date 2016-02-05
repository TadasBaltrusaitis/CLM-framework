% This is sort of the unit test for the whole module (needs datasets)
% Will take over an hour to run all

% TODO need some unit testy things, some asserts

tic
%% Head pose
cd('Head Pose Experiments');
run_clm_head_pose_tests_clnf;
assert(median(all_errors_biwi_ccnf_general(:)) < 2.7);
assert(median(all_errors_bu_ccnf_general(:)) < 2.2);
assert(median(all_errors_ict_ccnf_general(:)) < 2.1);
cd('../');

%% Features
cd('Feature Point Experiments');
run_clm_feature_point_tests_wild;
assert(median(err_clnf) < 0.041);
assert(median(err_clnf_wild) < 0.041);
run_yt_dataset;
assert(median(clnf_error) < 0.053);
cd('../');

%% AUs
cd('Action Unit Experiments');
run_AU_prediction_DISFA
assert(mean(au_res) > 0.6);
cd('../');

%% Gaze
cd('Gaze Experiments');
extract_mpii_gaze_test
assert(median_error < 9.5)
cd('../');

%% Demos
cd('Demos');
run_demo_images;
run_demo_videos;
run_demo_video_multi;
feature_extraction_demo_vid;
feature_extraction_demo_img_seq;
gaze_extraction_demo_vid;
cd('../');
toc