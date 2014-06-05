function top = nms_face(boxes,overlap)
% Non-maximum suppression.
% Greedily select high-scoring detections and skip detections
% that are significantly covered by a previously selected detection.

if nargin < 2
    overlap = 0.5;
end

N = length(boxes);

if isempty(boxes)
    top = [];
else
    numpart = size(boxes(1).xy,1);
    
    % throw away boxes with low score if there are too many candidates
    if N > 30000
        s = [boxes.s];
        [vals, I] = sort(s);
        boxes = boxes(I(end-29999:end));
    end
    N = min(30000,N);
    
    x1 = zeros(N,1);
    y1 = zeros(N,1);
    x2 = zeros(N,1);
    y2 = zeros(N,1);
    area = zeros(N,1);
    for nb = 1:N
        if numpart==1
            x1(nb) = boxes(nb).xy(1);
            y1(nb) = boxes(nb).xy(2);
            x2(nb) = boxes(nb).xy(3);
            y2(nb) = boxes(nb).xy(4);
        else
            x1(nb) = min(boxes(nb).xy(:,1));
            y1(nb) = min(boxes(nb).xy(:,2));
            x2(nb) = max(boxes(nb).xy(:,3));
            y2(nb) = max(boxes(nb).xy(:,4));
        end
        area(nb) = (x2(nb)-x1(nb)+1) * (y2(nb)-y1(nb)+1);
    end
    
    s = [boxes.s];
    [vals, I] = sort(s);
    pick = [];
    while ~isempty(I)
        last = length(I);
        i = I(last);
        pick = [pick; i];
        suppress = [last];
      
        j = I(1:last-1);
        xx1 = max(x1(i), x1(j));
        yy1 = max(y1(i), y1(j));
        xx2 = min(x2(i), x2(j));
        yy2 = min(y2(i), y2(j));
        
        w = xx2-xx1+1;
        w(w<0) = 0;
        h = yy2-yy1+1;
        h(h<0) = 0;
        
        inter = w.*h;
        o1 = inter ./ area(j);
        o2 = inter / area(i);
        idx =  (find((o1 > overlap) | (o2 > overlap)));
        suppress = [suppress ; idx];
        
        I(suppress) = [];
    end
    top = boxes(pick);
end
