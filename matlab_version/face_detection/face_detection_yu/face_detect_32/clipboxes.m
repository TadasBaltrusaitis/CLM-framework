function boxes = clipboxes(im, boxes)
% Clips boxes to image boundary.
imy = size(im,1);
imx = size(im,2);
for i = 1:length(boxes),
    b = boxes(i).xy;
    b(:,1) = max(b(:,1), 1);
    b(:,2) = max(b(:,2), 1);
    b(:,3) = min(b(:,3), imx);
    b(:,4) = min(b(:,4), imy);
    boxes(i).xy = b;
end