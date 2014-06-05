function Y = Select(X, rows) % x,y,z,x,y,z,...
    Y1 = X(3*(rows-1)+1, :);
    Y2 = X(3*(rows-1)+2, :);
    Y3 = X(3*(rows-1)+3, :);
    Y = [Y1; Y2; Y3];