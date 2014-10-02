function [ overlap ] = overlap( rect1, rect2 )
%OVERLAP Summary of this function goes here
%   Detailed explanation goes here

    dy1 = abs(rect1(1) - rect1(3)) + 1;
    dx1 = abs(rect1(2) - rect1(4)) + 1;
    dy2 = abs(rect2(1) - rect2(3)) + 1;
    dx2 = abs(rect2(2) - rect2(4)) + 1;
    
    a1 = dx1 * dy1;
    a2 = dx2 * dy2;
    ia = 0;
    
    if rect1(3) > rect2(1) && rect2(3) > rect1(1) && rect1(4) > rect2(2) && rect2(4) > rect1(2)
        
        xx1 = max(rect1(2), rect2(2));
        yy1 = max(rect1(1), rect2(1));
        xx2 = min(rect1(4), rect2(4));
        yy2 = min(rect1(3), rect2(3));
        ia = (xx2 - xx1 + 1) * (yy2 - yy1 + 1);
    end
    
    overlap = ia / double(a1 + a2 - ia);

end

