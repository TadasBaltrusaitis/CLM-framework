templateSize = 11; % square template

imgWidth = 320; % rectangular image
imgHeight = 240; 

template = rand(templateSize); % template with random entries
img = rand(imgHeight, imgWidth); % similarly for the image

% randomly embed the template inside the img
embedX = round(rand*(imgWidth-templateSize-1)+1);
embedY = round(rand*(imgHeight-templateSize-1)+1);
img( embedY:embedY+templateSize-1, embedX:embedX+templateSize-1 ) = template;

% display the input for normxcorr2
H = figure(1); clf;
set(H,'name','Can you find the template?');
subplot(1,2,1);
imagesc(img);
axis('image'); title('image');
subplot(1,2,2);
imagesc(template);
axis('image'); title('template');
colormap(gray);

% perform NCC
% choose 'same' as the output shape -- ie. zero-pad the output so 
% that it's the same size as the original image
% this is not necessary, but is used for display
ncc = normxcorr2_mex(template, img, 'same');
ncc1 = normxcorr2_mex(template, img);
ncc2 = normxcorr2(template, img);

% display the ncc
H = figure(2); clf;
set(H,'name','NCC can!');
imagesc(ncc);
axis('image'); title('normalized cross correlation');
colormap(gray);

% output where there template was embedded (centered)
fprintf('template was embedded at (%i,%i)\n', embedY+ceil((templateSize-1)/2), embedX+ceil((templateSize-1)/2));

% compute, then output where ncc 'found' the template (the highest score,
% which in this case is +1, since there is no noise)
[t mi] = max(ncc);
[t mj] = max(t);
mi = mi(mj);
fprintf('ncc found it at (%i,%i)\n', mi, mj);
