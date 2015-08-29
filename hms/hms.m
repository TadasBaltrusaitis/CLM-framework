clear
full_dir = 'C:\hms\full/';
process_dir = 'C:\hms\processed/';
out_dir = 'C:\hms\output_vids/';
if(~exist(out_dir, 'file'))
   mkdir(out_dir); 
end

sessions = dir([process_dir, '*MS*']);

command = 'C:\Users\Tadas\Documents\CLM-framework\Release\VisualiseVids.exe';

offsets = [616, 2003, -769, -1556, 9467, -2189, 613, -4972, 1679, -2625, -1933, -4053, -2791, -2567, -3526, -1064, 871, -2115, -2401, 316, 5731, 4206];

done = false(22,1);
done(4) = true;
done(5) = true;
done(6) = true;
done(7) = true;

sessions = sessions(~done);
offsets = offsets(~done);

parfor s = 1:numel(sessions)

    out_file = sprintf(' -ov "%s"', [out_dir, sessions(s).name, '.avi']);
    
    in_patient_vid_file = dir([full_dir, sessions(s).name, '/Participant/colour_*.avi']);
    in_patient_vid_file = sprintf(' -f "%s/%s/Participant/%s"', full_dir, sessions(s).name, in_patient_vid_file.name);

    in_patient_timestamp_file = dir([full_dir, sessions(s).name, '/Participant/colourLog_*.csv']);
    in_patient_timestamp_file = sprintf(' -t "%s/%s/Participant/%s"', full_dir, sessions(s).name, in_patient_timestamp_file.name);
    
    in_patient_pose_file = sprintf(' -ip "%s/%s/Participant_pose.txt"', process_dir, sessions(s).name);
    in_patient_fp_file = sprintf(' -if "%s/%s/Participant_fp.txt"', process_dir, sessions(s).name);
    in_patient_auc_file = sprintf(' -iac "%s/%s/Participant_auclass.txt"', process_dir, sessions(s).name);
    in_patient_aur_file = sprintf(' -iar "%s/%s/Participant_aureg.txt"', process_dir, sessions(s).name);
    
    in_doc_vid_file = dir([full_dir, sessions(s).name, '/Doctor/colour_*.avi']);
    in_doc_vid_file = sprintf(' -f "%s/%s/Doctor/%s"', full_dir, sessions(s).name, in_doc_vid_file.name);

    in_doc_timestamp_file = dir([full_dir, sessions(s).name, '/Doctor/colourLog_*.csv']);
    in_doc_timestamp_file = sprintf(' -t "%s/%s/Doctor/%s"', full_dir, sessions(s).name, in_doc_timestamp_file.name);
    
    in_doc_pose_file = sprintf(' -ip "%s/%s/Doctor_pose.txt"', process_dir, sessions(s).name);
    in_doc_fp_file = sprintf(' -if "%s/%s/Doctor_fp.txt"', process_dir, sessions(s).name);
    in_doc_auc_file = sprintf(' -iac "%s/%s/Doctor_auclass.txt"', process_dir, sessions(s).name);
    in_doc_aur_file = sprintf(' -iar "%s/%s/Doctor_aureg.txt"', process_dir, sessions(s).name);

    offset = sprintf(' -offset %d ', offsets(s));
    
    sum_doc = sprintf(' -osd "%s/%s_feats_doc.txt"', out_dir, sessions(s).name);
    sum_pat = sprintf(' -osp "%s/%s_feats_pat.txt"', out_dir, sessions(s).name);
    
    command_to_run = cat(2, command, out_file, in_patient_vid_file, in_patient_timestamp_file,...
                            in_patient_pose_file, in_patient_fp_file, in_patient_auc_file, in_patient_aur_file,...
                            in_doc_vid_file, in_doc_timestamp_file,...
                            in_doc_pose_file, in_doc_fp_file, in_doc_auc_file, in_doc_aur_file, offset, sum_doc, sum_pat);
                        
    dos(command_to_run);
    
end