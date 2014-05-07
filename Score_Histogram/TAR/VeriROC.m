clc
clear all
close all

%% verifinger match score load
% load('D:\matlabcode\evidentialvalue\nist14update.mat');
% load('D:\matlabcode\Morphoevidence\morphscore.mat');
% load('D:\matlabcode\Morphoevidence\scorefusion.mat')
%load('D:\matlabcode\Morphoevidence\scores.mat')
%load('D:\matlabcode\evidentialvalue\nist14')
load('C:\love\match_scores.mat')
gms1 = genpairs(:,1);
ims1 = imppairs(:,1);
gms2 = genpairs(:,2);
ims2 = imppairs(:,2);
gms3 = genpairs(:,3);
ims3 = imppairs(:,3);

% gms1 = verigen(:,1);
% ims1 = veriimp(:,1);
% gms2 = morpgen(:,1);
% ims2 = morpimp(:,1);
% gms3 = genpairs(:,1);
% ims3 = imppairs(:,1);
% gms4 = maxgenscore(:,1);
% ims4 = maximpscore(:,1);
% gms5 = sumgenscore(:,1);
% ims5 = sumimpscore(:,1);
% gms6 = progenscore(:,1);
% ims6 = proimpscore(:,1);
%FARs = (0.0001:0.0001:0.01);
FARs = (0.005:0.005:1);



%% calculate FAR and TAR
[TARs1,FARs] = TARatFAR(gms1,ims1,FARs);
[TARs2,FARs] = TARatFAR(gms2,ims2,FARs);
[TARs3,FARs] = TARatFAR(gms3,ims3,FARs);
% [TARs4,FARs] = TARatFAR(gms4,ims4,FARs);
% [TARs5,FARs] = TARatFAR(gms5,ims5,FARs);
% [TARs6,FARs] = TARatFAR(gms6,ims6,FARs);

GGG = 1- FARs;
figure; plot(FARs, TARs1,'g');
hold on
plot(FARs, TARs2,'r');
hold on
plot(FARs, TARs3,'b');
hold on;
plot(FARs, GGG, 'c');
% hold on
% plot(FARs, TARs4,'m');
% hold on
% plot(FARs, TARs5,'b');
% hold on
% plot(FARs, TARs6,'c');
% hold on; plot(FARs,1-FARs);
xlabel('False Acceptance Rate');
ylabel('True Acceptance Rate');
grid on;
% set(gca, 'XScale', 'log');
% set(gca, 'YScale', 'log');
