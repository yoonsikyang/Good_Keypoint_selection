function [TARs,FARs] = TARatFAR(gms,ims,FARs)
% TARatFAR
%
% Jianjiang Feng
% 2009-07

sorted_ims = sort(ims);
len_gm = length(gms);
len_im = length(ims);
FAR_points = unique(round(len_im*FARs));
FARs = FAR_points/len_im;
TARs = zeros(1,length(FAR_points));
for i = 1:length(FAR_points)
    TARs(i) = (length(find(gms > sorted_ims(len_im+1-FAR_points(i)))))/len_gm;
end
