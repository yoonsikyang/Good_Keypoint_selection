clc
close all
clear all
%% data load
%load('E:\nist14update.mat')
load('C:\love\match_scores.mat')

% match score extraction
genpairs = genpairs(:,3);
imppairs = imppairs(:,3);

% total num of pairs
totalgen = length(genpairs);
totalimp = length(imppairs);

% min and max score
minscore =0;
maxscore = max(genpairs);
score = (minscore:1:maxscore)';

%% calculate distribution
gendistr = histc(genpairs, minscore:1:maxscore);
impdistr = histc(imppairs, minscore:1:maxscore);

gendistr = gendistr./totalgen;
impdistr = impdistr./totalimp;

windowlength = 3;
kernel = ones(1,windowlength)';
kernel = kernel/windowlength;

% gensmoo = conv(gendistr,kernel,'same');
% gensmoo(1) = gendistr(1);
% gensmoo(end) = gendistr(end);
% 
% impsmoo = conv(impdistr,kernel,'same');
% impsmoo(1) = impdistr(1);
% impsmoo(end) = impdistr(end);

%% plot
plot(score, gendistr,'r'); hold on;
plot(score, impdistr,'b');