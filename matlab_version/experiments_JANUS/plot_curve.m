function plot_curve( shape, varargin)
%PLOT_CURVE Summary of this function goes here
%   Detailed explanation goes here

    % construct two sets, upper and lower based on minimum and maximum
    % values of x

    x = shape(:,1);
    y = shape(:,2);

    [~, ind_min] = min(x);
    [~, ind_max] = max(x);
    
    if(ind_min < ind_max)
       x1 = x(ind_min:ind_max);
       y1 = y(ind_min:ind_max);

       x2 = cat(1, x(ind_max:end), x(1:ind_min));
       y2 = cat(1, y(ind_max:end), y(1:ind_min));  

    else
       x1 = cat(1, x(ind_min:end), x(1:ind_max));
       y1 = cat(1, y(ind_min:end), y(1:ind_max));
              
       x2 = x(ind_max:ind_min);
       y2 = y(ind_max:ind_min);
    end

    n = numel(x1); % number of original points
    xi = interp1( 1:n, x1, linspace(1, n, 10*n) ); % new sample points 
    yi = interp1(   x1, y1, xi, 'pchip' );

    plot( xi, yi, varargin{:} ); % should be smooth between the original point

    n = numel(x2); % number of original points
    xi = interp1( 1:n, x2, linspace(1, n, 10*n) ); % new sample points 
    yi = interp1(   x2, y2, xi, 'pchip' );

    plot( xi, yi, varargin{:} ); % should be smooth between the original point

end

