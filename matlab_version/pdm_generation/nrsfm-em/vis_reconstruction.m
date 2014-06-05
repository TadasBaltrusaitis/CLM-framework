function vis_reconstruction(P3_gt, P3_rec)

[T, J] = size(P3_gt); T = T/3;

xmin = min(min(P3_gt(1:T,:)));
xmax = max(max(P3_gt(1:T,:)));
xmin = xmin - (xmax-xmin)*0.1;
xmax = xmax + (xmax-xmin)*0.1;
ymin = min(min(P3_gt(T+1:2*T,:)));
ymax = max(max(P3_gt(T+1:2*T,:)));
ymin = ymin - (ymax-ymin)*0.1;
ymax = ymax + (ymax-ymin)*0.1;
zmin = min(min(P3_gt(2*T+1:3*T,:)));
zmax = max(max(P3_gt(2*T+1:3*T,:)));
zmin = zmin - (zmax-zmin)*0.1;
zmax = zmax + (zmax-zmin)*0.1;

figure(3);
hold off;
plot([xmin xmax], [ymin ymin], 'k-');
hold on;
axis equal;
axis off;
set(gcf, 'color', [1 1 1]);
plot([xmin xmax], [ymax ymax], 'k-');
plot([xmin xmin], [ymin ymax], 'k-');
plot([xmax xmax], [ymin ymax], 'k-');
delta = ymin-zmax-10;
plot([xmin xmax], [zmin zmin]+delta, 'k-');
plot([xmin xmax], [zmax zmax]+delta, 'k-');
plot([xmin xmin], [zmin zmax]+delta, 'k-');
plot([xmax xmax], [zmin zmax]+delta, 'k-');
ht1=text((xmin+xmax)/2, ymax-10, 'Input 2D tracks', 'HorizontalAlignment', 'center', 'fontweight', 'bold', 'fontname', 'verdana');
ht2=text((xmin+xmax)/2, zmax+delta-10, '3D shape', 'HorizontalAlignment', 'center', 'fontweight', 'bold', 'fontname', 'verdana');


for t=1:T
   if t>1,
      delete(hs1);
      delete(hs2);
      delete(hs3);
   end
   hs1 = plot(P3_gt(t,:), P3_gt(t+T,:), 'g.');   
   hs2 = plot(P3_gt(t,:), P3_gt(t+2*T,:)-mean(P3_gt(t+2*T,:))+delta, 'b.');
   hs3 = plot(P3_rec(t,:), P3_rec(t+2*T,:)-mean(P3_rec(t+2*T,:))+delta, 'ro');   
   if t==1,
      hh=legend([hs2,hs3],'ground truth', 'reconstruction', 4);
      set(hh, 'fontname', 'verdana');
   end
   drawnow;
   
   I = getframe;
   
   if 0,
      str = sprintf('frame%04d', t);
      imwrite(I.cdata, [str '.jpg'], 'Quality', 100);      
   end
end

