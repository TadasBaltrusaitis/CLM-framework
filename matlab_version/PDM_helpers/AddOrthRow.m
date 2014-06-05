function [RotFull] = AddOrthRow(RotSmall)

  % We can work out these values from the small version of the rotation matrix Rx * Ry * Rz (if you plug in values you can work it out, just slightly tedious)
  RotFull = zeros(3,3);
  RotFull(1:2, :) = RotSmall;
  RotFull(3,1) = RotSmall(1, 2) * RotSmall(2, 3) - RotSmall(1, 3) * RotSmall(2, 2);
  RotFull(3,2) = RotSmall(1, 3) * RotSmall(2, 1) - RotSmall(1, 1) * RotSmall(2, 3);
  RotFull(3,3) = RotSmall(1, 1) * RotSmall(2, 2) - RotSmall(1, 2) * RotSmall(2, 1);
  