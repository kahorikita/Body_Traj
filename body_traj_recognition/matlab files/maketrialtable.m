%make full set of trial tables for Same-Different Gesture test

%trial table: 
%trialType vid1 vid2 vidtype cresp srdur trdur practice
% trialType:    Flag for trial type ( 1 = Same/Different test )
% vid1:         First video file
% vid2:         Second video file
% vidtype:      0 = same; 1 = different body config; 2 = different trajectory; 3 = different both
% cresp:        correct response (0 = same, 1 = different)
% srdur:        duration to wait after video playback before go cue (stim-response delay duration)
% trdur:        trial duration (max response time)
% practice      0 = test, 1 = practice
curdir = cd;
cd ../TrialTables

%--------------------------------------------------------------------------

%for each gesture there are a pair of videos that share 1 attribute. So we
%can have 2 same, 4 different (we will limit to 2), and 2
%one-attribute-same combinations, for a total of 6 unique video pairs.
%we will test half the combinations  (1 same, 1 diff, 1 one-attrib) per
%block.

Npractice = 2;  %number of practice gestures; assume there are (Npractice*2) videos, so we can make (Npractice*6) trials
Ntrain = 12;    %number of test gestures; assume there are (Ntrain*2) videos, so we can make (Ntrain*6) unique trials

SRdur = 800;
TRdur = 2000;
PauseTrial = 0; %set to 1 to pause between trials for experimenter input; set to 0 to automatically progress to next trial

rng('shuffle');

%--------------------------------------------------------------------------

%we will generate 2 trial tables per person (one per session). for the
%  practice trials we will just redo the randomization process twice. for
%  the test trial table we will split the cases into two so that there are
%  paired but non-overlapping stimuli in each session
%

for itbl = 1:2
    %Practice Same-Different
    trial = [];
    quit = 0;
    
    while quit ~= 1
        
        %same - 2 trials
        for a = 1:Npractice
            vidtype = 0;
            g1 = a; %gesture number for the first video
            g2 = a; %gesture number for the second video
            %offset for practice videos
            g1 = g1+12;
            g2 = g2+12;
            
            p1 = randi(2)-1; %perspective for the first video
            p2 = setxor([0:1],p1); %perspective for the second video, pick the other one
            s1 = randi(2)-1; %of the two paired share-one-attribute vidoes, pick one (0-1)
            s2 = s1; %of the two paired share-one-attribute vidoes, pick the same one
            
            vid1 = (g1-1)*4 + (s1)*2 + p1; %video number: gesture number + one-attrib-share number + perspective number
            vid2 = (g2-1)*4 + (s2)*2 + p2; %for "same" videos, everything but the perspective number should be the same
            
            %randomly choose whether g1 or g2 goes first
            if rand > 0.5
                tmp = vid1;
                vid1 = vid2;
                vid2 = tmp;
            end
            
            %trialType vid1 vid2 vidtype cresp srdur trdur practice pause
            trial = [trial; 1 vid1 vid2 vidtype 0 SRdur TRdur 1 1];
            %fprintf(fid,'1 %d %d %d 0 %d %d 1\n',vid1,vid2,vidtype,SRdur,TRdur);
        end
        
        %different
        for a = 1:Npractice
            vidtype = 3;
            g1 = a; %gesture number for the first video
            g2 = setxor([1:Npractice],g1); %gesture number for the second video - any of the other gestures
            g2 = g2(1);
            %offset for practice videos
            g1 = g1+12;
            g2 = g2+12;
            
            p1 = randi(2)-1; %perspective for the first video
            p2 = setxor([0:1],p1); %perspective for the second video, pick the other one
            s1 = randi(2)-1; %of the two paired share-one-attribute vidoes, pick any one (0-1)
            s2 = randi(2)-1; %of the two paired share-one-attribute vidoes, pick any one (0-1)
            
            vid1 = (g1-1)*4 + (s1)*2 + p1; %video number: gesture number + one-attrib-share number + perspective number
            vid2 = (g2-1)*4 + (s2)*2 + p2; %for "same" videos, everything but the perspective number should be the same
            
            %randomly choose whether g1 or g2 goes first
            if rand > 0.5
                tmp = vid1;
                vid1 = vid2;
                vid2 = tmp;
            end
            
            %trialType vid1 vid2 vidtype cresp srdur trdur practice pause
            trial = [trial; 1 vid1 vid2 vidtype 0 SRdur TRdur 1 1];
            %fprintf(fid,'1 %d %d %d 1 %d %d 1\n',vid1,vid2,vidtype,SRdur,TRdur);
        end
        
        
        %different - one-attribute
        for a = 1:Npractice
            
            if a < (Npractice/2)
                vidtype = 1;
            else
                vidtype = 0;
            end
            g1 = a; %gesture number for the first video
            g2 = a; %gesture number for the second video
            %offset for practice videos
            g1 = g1+12;
            g2 = g2+12;
            
            p1 = randi(2)-1; %perspective for the first video
            p2 = setxor([0:1],p1); %perspective for the second video, pick the other one
            s1 = randi(2)-1; %of the two paired share-one-attribute vidoes, pick one (0-1)
            s2 = setxor([0 1],s1); %of the two paired share-one-attribute vidoes, pick the other one
            
            vid1 = (g1-1)*4 + (s1)*2 + p1; %video number: gesture number + one-attrib-share number + perspective number
            vid2 = (g2-1)*4 + (s2)*2 + p2; %for "same" videos, everything but the perspective number should be the same
            
            %randomly choose whether g1 or g2 goes first
            if rand > 0.5
                tmp = vid1;
                vid1 = vid2;
                vid2 = tmp;
            end
            
            %trialType vid1 vid2 vidtype cresp srdur trdur practice pause
            trial = [trial; 1 vid1 vid2 vidtype 0 SRdur TRdur 1 1];
        end
        
        x = reshape(trial(:,2:3),[],1);
        y = hist(x,unique(x));
        
        if all(y) <= Npractice
            quit = 1;
        end
        
    end
    
    fid = fopen(sprintf('P_SameDiffGest%d.txt',itbl),'wt');
    
    for a = 1:size(trial,1)
        fprintf(fid,'%d ',trial(a,:));
        fprintf(fid,'\n');
    end
    
    fclose(fid);
end




%Test Same-Different
trial1 = [];
trial2 = [];

%all same pairings
vidsame = [0:4:47; 1:4:47;  2:4:47; 3:4:47]';

%we will randomly pick half of the share1 pairs for each session
iones = round(size(vidsame,1)/2);
vidsamepairs = [ones(iones,1); zeros(size(vidsame,1)-iones,1)];
while sum(~(vidsamepairs(3:end)-vidsamepairs(2:end-1)) & ~(vidsamepairs(3:end)-vidsamepairs(1:end-2))) ~= 0 %make sure there are no 3-in-a-row of the same type
    vidsamepairs = vidsamepairs(randperm(size(vidsamepairs,1)));
end

for a = 1:length(vidsamepairs)
    if vidsamepairs(a) == 1
        trial1 = [trial1; vidsame(a,1:2) 0 0];
        trial2 = [trial2; vidsame(a,3:4) 0 0];
    else
        trial1 = [trial1; vidsame(a,3:4) 0 0];
        trial2 = [trial2; vidsame(a,1:2) 0 0];
    end
end



%all one-attrib-share pairings
%we assume the first 6 videos share the same trajectory, while the last 6
%videos share the same body configuration
vidonea = [0:4:47; 3:4:47; 1:4:47; 2:4:47; [ones(1,6) 2*ones(1,6)]]';

%we will randomly pick half of the share1 pairs for each session
iones = round(size(vidonea,1)/2);
vidoneapairs = [ones(iones,1); zeros(size(vidonea,1)-iones,1)];
while sum(~(vidoneapairs(3:end)-vidoneapairs(2:end-1)) & ~(vidoneapairs(3:end)-vidoneapairs(1:end-2))) ~= 0 %make sure there are no 3-in-a-row of the same type
    vidoneapairs = vidoneapairs(randperm(size(vidoneapairs,1)));
end
for a = 1:length(vidoneapairs)
    if vidoneapairs(a) == 1
        trial1 = [trial1; vidonea(a,1:2) vidonea(a,5) 1];
        trial2 = [trial2; vidonea(a,3:4)  vidonea(a,5) 1];
    else
        trial1 = [trial1; vidonea(a,3:4)  vidonea(a,5) 1];
        trial2 = [trial2; vidonea(a,1:2)  vidonea(a,5) 1];
    end
end



%all different pairings - we will pick them randomly after removing all the
% videos that are already chosen twice

vids1 = reshape(trial1(:,1:2),[],1);
uv = unique(vids1);
count1  = histc(vids1,uv);
remvids = uv(count1>1);
allvids = 0:47;
diffvids1 = allvids;
%goodvids = setxor(allvids,vids(count > 1));
%allvids = [0:4:47; 1:4:47;  2:4:47; 3:4:47]';
diffvids1(remvids+1) = NaN; %remove all the videos already used twice in this set
diffvids1 = reshape(diffvids1,4,[])';

vids2 = reshape(trial2(:,1:2),[],1);
uv = unique(vids2);
count2  = histc(vids2,uv);
remvids = uv(count2>1);
diffvids2 = allvids;
%goodvids = setxor(allvids,vids(count > 1));
%allvids = [0:4:47; 1:4:47;  2:4:47; 3:4:47]';
%diffvids2(vids2(count2 > 1)+1) = NaN; %remove all the videos already used twice in this set
diffvids2(remvids+1) = NaN; %remove all the videos already used twice in this set
diffvids2 = reshape(diffvids2,4,[])';
allvids = reshape(allvids,4,[])';

%choose which of the two pairs of movements will be used; we want an even number of both
iones = round(size(allvids,1)/2);
viddiffpairs1 = [ones(iones,1); zeros(size(vidonea,1)-iones,1)];

while sum(~(viddiffpairs1(3:end)-viddiffpairs1(2:end-1)) & ~(viddiffpairs1(3:end)-viddiffpairs1(1:end-2))) > 1
    
    for a = 1:size(diffvids1,1)
        if any(isnan(diffvids1(a,1:2)))
            viddiffpairs1(a) = 1;
        elseif any(isnan(diffvids1(a,3:4)))
            viddiffpairs1(a) = 0;
        elseif ~any(isnan(diffvids1(a,:))) %all are good, pick any one at random
            viddiffpairs1(a) = round(rand);
        else
            viddiffpairs1(a) = NaN;
        end
    end
end

iones = round(size(allvids,1)/2);
viddiffpairs2 = [ones(iones,1); zeros(size(vidonea,1)-iones,1)];

while sum(~(viddiffpairs2(3:end)-viddiffpairs2(2:end-1)) & ~(viddiffpairs2(3:end)-viddiffpairs2(1:end-2))) > 1
    
    for a = 1:size(diffvids2,1)
        if any(isnan(diffvids2(a,1:2)))
            viddiffpairs2(a) = 1;
        elseif any(isnan(diffvids2(a,3:4)))
            viddiffpairs2(a) = 0;
        elseif ~any(isnan(diffvids2(a,:))) %all are good, pick any one at random
            viddiffpairs2(a) = round(rand);
        else
            viddiffpairs2(a) = NaN;
        end
    end
end

%of the pairs, choose one of them to be the primary video
overlap = 1;
while overlap
    
    for a = 1:length(viddiffpairs1)
        if viddiffpairs1(a) == 1
            vid1(a) = allvids(a,randi(2,1)+2);
        else
            vid1(a) = allvids(a,randi(2,1));
        end
    end
    
    for a = 1:length(viddiffpairs2)
        if viddiffpairs2(a) == 1
            vid2(a) = allvids(a,randi(2,1)+2);
        else
            vid2(a) = allvids(a,randi(2,1));
        end
    end
    
    if length(intersect(vid1,vid2)) < size(allvids,1)/5
        overlap = 0;
    end
end

%now that we've picked video 1, we need to select video 2 from the
% remaining videos at random, while ensuring that we DON'T pick the same
% stimulus pair and we DO pick the opposing perspective, while keeping
% track of which videos we pick so we don't ever show the same video twice
% per block
alldiffvids1 = diffvids1;
diffvids1 = reshape(diffvids1',[],1);
diffvids1(vid1+1) = NaN;
diffvids1(vid2+1) = NaN;
diffvids1 = reshape(diffvids1,4,[])';
%diffvids1(isnan(diffvids1)) = [];
tmpdiffvids = diffvids1;
difftrial1 = [];
for a = 1:size(diffvids1,1)
    tmp = vid1(a);
    
    [ia,ib] = find(alldiffvids1 == tmp);       %remove all the same perspective and same stimulus pair videos
    tmp2 = diffvids1;
    tmp2(ia,:) = [];
    if ib <= 2
        tmp2(:,[ib ib+2]) = [];
    else
        tmp2(:,[ib-2 ib]) = [];
    end
    tmp2(isnan(tmp2)) = [];
    tmp2 = tmp2(randperm(length(tmp2)));    %randomly select one of the remaining videos
    [ia2,ib2] = find(diffvids1 == tmp2(1));
    %futurediffvids1 = diffvids1(ia:end,:);
    
    while (ia2 >= ia) && sum(~isnan(diffvids1(ia2,:))) <= 1 %ensure that there is at least 1 other video that could be selected in that stimulus pair, for pairs > the current tmp under consideration
        tmp2 = tmp2(randperm(length(tmp2))); 
        [ia2,ib2] = find(diffvids1 == tmp2(1));
    end
    
    difftrial1 = [difftrial1; [tmp(1) tmp2(1) 3 1]];
    [ia,ib] = find(diffvids1 == tmp(1));
    diffvids1(ia,ib) = NaN;
    [ia,ib] = find(diffvids1 == tmp2(1));
    diffvids1(ia,ib) = NaN;
    
end
diffvids1 = tmpdiffvids;

alldiffvids2 = diffvids2;
diffvids2 = reshape(diffvids2',[],1);
diffvids2(vid2+1) = NaN;
diffvids2(vid1+1) = NaN;
diffvids2 = reshape(diffvids2,4,[])';
%diffvids1(isnan(diffvids1)) = [];
tmpdiffvids = diffvids2;
difftrial2 = [];
for a = 1:size(diffvids2,1)
    tmp = vid2(a);
    
    [ia,ib] = find(alldiffvids2 == tmp);       %remove all the same perspective and same stimulus pair videos
    tmp2 = diffvids2;
    tmp2(ia,:) = [];
    if ib <= 2
        tmp2(:,[ib ib+2]) = [];
    else
        tmp2(:,[ib-2 ib]) = [];
    end
    tmp2(isnan(tmp2)) = [];
    tmp2 = tmp2(randperm(length(tmp2)));    %randomly select one of the remaining videos
    [ia2,ib2] = find(diffvids2 == tmp2(1));
    %futurediffvids1 = diffvids1(ia:end,:);
    
    while (ia2 >= ia) && sum(~isnan(diffvids2(ia2,:))) <= 1 %ensure that there is at least 1 other video that could be selected in that stimulus pair, for pairs > the current tmp under consideration
        tmp2 = tmp2(randperm(length(tmp2))); 
        [ia2,ib2] = find(diffvids2 == tmp2(1));
    end
    
    difftrial2 = [difftrial2; [tmp(1) tmp2(1) 3 1]];
    [ia,ib] = find(diffvids2 == tmp(1));
    diffvids2(ia,ib) = NaN;
    [ia,ib] = find(diffvids2 == tmp2(1));
    diffvids2(ia,ib) = NaN;
    
end
diffvids2 = tmpdiffvids;


%create the full trial table structure
trial1 = [trial1; difftrial1];
trial2 = [trial2; difftrial2];

order1 = randperm(size(trial1,1));
order2 = randperm(size(trial2,1));

trial1 = trial1(order1,:);
trial2 = trial2(order1,:);

%trialType vid1 vid2 vidtype cresp srdur trdur practice
trial1 = [ones(size(trial1,1),1) trial1 SRdur*ones(size(trial1,1),1) TRdur*ones(size(trial1,1),1) zeros(size(trial1,1),1) PauseTrial*ones(size(trial1,1),1)];
trial2 = [ones(size(trial2,1),1) trial2 SRdur*ones(size(trial2,1),1) TRdur*ones(size(trial2,1),1) zeros(size(trial2,1),1) PauseTrial*ones(size(trial1,1),1)];

%print the trial tables out to files

fid = fopen('SameDiffGest1.txt','wt');

for a = 1:size(trial1,1)
    fprintf(fid,'%d ',trial1(a,:));
    fprintf(fid,'\n');
end

fclose(fid);

fid = fopen('SameDiffGest2.txt','wt');

for a = 1:size(trial2,1)
    fprintf(fid,'%d ',trial2(a,:));
    fprintf(fid,'\n');
end

fclose(fid);

