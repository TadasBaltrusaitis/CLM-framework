function Tr = mstep_update_transl(P, S_bar, V, E_z, RO)
% Tr = mstep_update_transl(P, S_bar, V, E_z, RO)

% Updates translation using Eq 23

[K, T] = size(E_z);
J = size(S_bar, 2);

Tr = zeros(T, 2);
for t = 1:T,    
   Sdef = S_bar;
   for kk = 1:K,
      Sdef = Sdef + E_z(kk,t)*V((kk-1)*3+[1:3],:);
   end;
   
   R_t = RO{t};
   XY = R_t(1:2,:)*Sdef;
   
   t_t = sum(P([t t+T], :) - XY, 2)./J;
   
   Tr(t, :) = t_t';
end
