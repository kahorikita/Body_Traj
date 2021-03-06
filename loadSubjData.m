% function [data] = loadSubjData(subjname)
% load a single subject's timed response target jump data
% data = loadSubjData('Data/S01',{'B1','B2','B3','B4','B5'});

% IMT002: F=PMd, R=SMG
% IMT003: F=SMG, R=PMd

clear;
clc;

c_same = 1;
c_diffb = 1;
c_difft = 1;
c_diff = 1;

for session = 1:6
    
    if session == 1
        %%% IMT002
%         subjname = 'IMT002/IMT002_SameDiffGestF1_002_20200220194229.dat';
%         subjname = 'IMT002/IMT002_SameDiffGestR1_002_20200220202755.dat';

        %%% IMT003
%         subjname = 'IMT003/IMT003_SameDiffGestF1_1_003_20200225144256.dat';
        subjname = 'IMT003/IMT003_SameDiffGestR1_1_003_20200225153152.dat';

    elseif session == 2
        %%% IMT002
%         subjname = 'IMT002/IMT002_SameDiffGestF2_1_003_20200221141849.dat';
%         subjname = 'IMT002/IMT002_SameDiffGestR2_1_003_20200221134051.dat';

        %%% IMT003
%         subjname = 'IMT003/IMT003_SameDiffGestF1_2_003_20200225150146.dat';
        subjname = 'IMT003/IMT003_SameDiffGestR1_2_003_20200225154052.dat';
        
    elseif session == 3
        %%% IMT002
%         subjname = 'IMT002/IMT002_SameDiffGestF2_2_003_20200221144137.dat';
%         subjname = 'IMT002/IMT002_SameDiffGestR2_2_003_20200221140438.dat';

        %%% IMT003
%         subjname = 'IMT003/IMT003_SameDiffGestF1_3_003_20200225151417.dat';
        subjname = 'IMT003/IMT003_SameDiffGestR1_3_003_20200225155337.dat';

    elseif session == 4 
        %%% IMT003
%         subjname = 'IMT003/IMT003_SameDiffGestF2_1_003_20200226120540.dat';
        subjname = 'IMT003/IMT003_SameDiffGestR2_1_003_20200226113436.dat';

    elseif session == 5
        %%% IMT003
%         subjname = 'IMT003/IMT003_SameDiffGestF2_2_003_20200226122815.dat';
        subjname = 'IMT003/IMT003_SameDiffGestR2_2_003_20200226115231.dat';
        
    elseif session == 6
        % IMT003
%         subjname = 'IMT003/IMT003_SameDiffGestF2_3_003_20200226123119.dat';
        subjname = 'IMT003/IMT003_SameDiffGestR2_3_003_20200226120211.dat';

    end
    disp(subjname);
    d = dlmread(subjname,'',11,1);
    
    % 1.Time 2.Trial 3.Redo 4.Vid1 5.Vid2 6.VidType 7.VidStatus 8.Instruct 9.TMS
    % 10.CResp 11.Resp 12.Correct 13.Score 14.Practice 15.Latency
    %
    % 6.VidType
    % 0=same, 1=different body config, 2=different trajectory, 3=different both
    
    Ntrials = length(d(:,1));
    Vidtype = d(:,6);
    TMS = d(:,9); %1=on, -1=off
    Correct = d(:,12); %1=answer is correct, 0=wrong
    RT = d(:,15);
    
    
    for trial = 1:Ntrials
        
        if Vidtype(trial) == 0 && RT(trial) < 500 %same
            Pat{1}(c_same,1) = trial;
            Pat{1}(c_same,2) = Correct(trial);
            Pat{1}(c_same,3) = TMS(trial);
            Pat{1}(c_same,4) = RT(trial);
            c_same = c_same + 1;
        elseif Vidtype(trial) == 1 && RT(trial) < 500 %different body config
            Pat{2}(c_diffb,1) = trial;
            Pat{2}(c_diffb,2) = Correct(trial);
            Pat{2}(c_diffb,3) = TMS(trial);
            Pat{2}(c_diffb,4) = RT(trial);
            c_diffb = c_diffb + 1;
        elseif Vidtype(trial) == 2 && RT(trial) < 500 %different trajectory
            Pat{3}(c_difft,1) = trial;
            Pat{3}(c_difft,2) = Correct(trial);
            Pat{3}(c_difft,3) = TMS(trial);
            Pat{3}(c_difft,4) = RT(trial);
            c_difft = c_difft + 1;
        elseif Vidtype(trial) == 3 && RT(trial) < 500 %different both
            Pat{4}(c_diff,1) = trial;
            Pat{4}(c_diff,2) = Correct(trial);
            Pat{4}(c_diff,3) = TMS(trial);
            Pat{4}(c_diff,4) = RT(trial);
            c_diff = c_diff + 1;
        end
        
    end

end 

c_same = c_same - 1;
c_diffb = c_diffb - 1;
c_difft = c_difft - 1;
c_diff = c_diff - 1;

for num=1:4
    TMS_accuracy(num,1) = sum(Pat{num}(Pat{num}(:,3)==1,2))/length(Pat{num}(Pat{num}(:,3)==1,2)); % correct rate, TMS on 
    TMS_accuracy(num,2) = sum(Pat{num}(Pat{num}(:,3)==-1,2))/length(Pat{num}(Pat{num}(:,3)==-1,2)); % correct rate, TMS off
end


for num = 1:4
    temp = Pat{num};
    temp(temp(:,2)==0,:) = [];
    TMS_RT(num,1) = mean(temp(temp(:,3)==1,4)); % mean RT, TMS on, correct answers
    TMS_RT(num,2) = std(temp(temp(:,3)==1,4)); % std RT, TMS on, correct answers
    TMS_RT(num,3) = mean(temp(temp(:,3)==-1,4)); % mean RT, TMS off, correct answers
    TMS_RT(num,4) = std(temp(temp(:,3)==-1,4)); % std RT, TMS off, correct answers

%     TMS_RT(num,1) = mean(Pat{num}(Pat{num}(:,3)==1,4)); % mean RT, TMS on, correct answers
%     TMS_RT(num,2) = mean(Pat{num}(Pat{num}(:,3)==-1,4)); % mean RT, TMS off, correct answers     
end


% save 'PMd.mat' TMS_RT TMS_accuracy 

% data.all=d; 

% Nblocks = length(blocknames);
% trial = 1;
% tFileFull = [];
% 
% for blk=1:Nblocks
% %     disp(['SameDiffGest3 ',subjname,', ','Block: ',blocknames(blk)]);
%     disp(['SameDiffGest3 ',subjname,', ','Block: ',blocknames(blk)]);
%     path = [subjname,'/',blocknames{blk}];
%     disp(path);
% %     tFile = dlmread([path,'/tFile.tgt'],' ',0,0);
% %     fnames = dir(path);
% %     Ntrials = size(tFile,1);
% %             if(strcmp(fnames(3).name,'.DS_Store'))
% %                 istart=3;
% %             else
% %                 istart=2;
% %             end
%     Ntrials = 34;
%     for j=1:Ntrials
% 
%         d = dlmread(['SameDiffGest3_0001_20200128132920',fnames(j+istart).name],' ',6,0);
% 
% %         L{trial} = d(:,1:2); % left hand X and Y
% %         R{trial} = d(:,3:4); % right hand X and Y
% %         C{trial} = d(:,5:6);% cursor X and Y
% %         %N{trial} = [L{trial}(:,1) R{trial}(:,2)]; % null space movements
% % 
% %         % absolute target location
% %         targetAbs(trial,1) = tFile(j,2)+START_X;
% %         targetAbs(trial,2) = tFile(j,3)+START_Y;
% %        
% % 
% %         % for target jump
% %         % if targets are on x/y axis
% %         if (tFile(j,3)==0.08 && tFile(j,4)==0.05) || (tFile(j,3)==-0.08 && tFile(j,4)==0.05) % target is shown on y axis, target jump +x
% %             tFile(j,6)=1;     
% %         elseif (tFile(j,3)==0.08 && tFile(j,4)==-0.05) || (tFile(j,3)==-0.08 && tFile(j,4)==-0.05) % target is shown on y axis, target jump -x
% %             tFile(j,6)=2;
% %         elseif (tFile(j,2)==0.08 && tFile(j,4)==0.05 || tFile(j,2)==-0.08 && tFile(j,4)==0.05) % target is shown on x axis, target jump +y
% %             tFile(j,6)=3;
% %         elseif (tFile(j,2)==0.08 && tFile(j,4)==-0.05 || tFile(j,2)==-0.08 && tFile(j,4)==-0.05) % target is shown on x axis, target jump -y
% %             tFile(j,6)=4;
% % 
% % %         elseif (tFile(j,2)==0.08 && tFile(j,4)==0.05) % target is shown on +x axis, target jump +y
% % %             tFile(j,6)=3;
% % %         elseif (tFile(j,2)==-0.08 && tFile(j,4)==0.05) % target is shown on -x axis, target jump +y
% % %             tFile(j,6)=4;
% % %         elseif (tFile(j,2)==0.08 && tFile(j,4)==-0.05) % target is shown on +x axis, target jump -y
% % %             tFile(j,6)=5;
% % %         elseif (tFile(j,2)==-0.08 && tFile(j,4)==-0.05) % target is shown on -x axis, target jump -y
% % %             tFile(j,6)=6;
% %         end
% % 
% %         % if targets are on 45 degrees
% % %         if tFile(j,4)==0.03 % target jump to + direction
% % %             tFile(j,6)=1;
% % %         elseif tFile(j,4)==-0.03 % target jump to - direction
% % %             tFile(j,6)=2;
% % %         end
% % 
% %         
% %         % determine relative target location
% %         %if(j>1)
% %         %    start(trial,:) = targetAbs(trial-1,:);    
% %         %else
% %             start(trial,:) = [START_X START_Y];
% %         %end
% %         targetRel(trial,:) = targetAbs(trial,:)-start(trial,:);
% %         %pert(trial) = tFile(j,4);
% %         pert(trial) = tFile(j,6);
% %         
% %         ip = find(d(:,8));
% %         if(isempty(ip))
% %            ipertonset(trial) = NaN; % time of perturbation onset
% %         else
% %            ipertonset(trial) = min(ip);
% %         end
% %         
% %         imov = find(d(:,7)==4); % time of movement onset
% %         if(isempty(imov))
% %             imoveonset = 1;
% %         else
% %             imoveonset(trial) = min(imov);
% %         end        
% %         state{trial} = d(:,7); % trial 'state' at each time point
% %         time{trial} = d(:,9); % time during trial
% %         
% %         trial = trial+1;
% %         
%     end
% %     
% %     tFileFull = [tFileFull; tFile(:,1:6)]; % copy of trial table
% %     % number of hit in each block
% %     hitcount(blk,1) = length(find(tFile(:,6))==1);
% %     
% end
% %Lc = L;
% %Rc = R;
% 
% % % compute target angle
% % data.targAng = atan2(targetRel(:,2),targetRel(:,1));
% % data.targDist = sqrt(sum(targetRel(:,1:2)'.^2));
% % 
% % % store all info in data structure 'data'
% % data.L = L;
% % data.R = R;
% % data.C = C;
% % %data.N = N;
% % 
% % data.Ntrials = size(targetRel,1);
% % data.tFile = tFileFull;
% % data.pert = pert;
% % 
% % data.state = state;
% % data.time = time;
% % data.ipertonset = ipertonset;
% % data.imoveonset = imoveonset;
% % 
% % data.subjname = subjname;
% % data.blocknames = blocknames;
% % 
% % % placeholders - these will be computed later
% % d0 = 0;
% % data.rPT = d0;
% % data.reachDir = d0;
% % data.d_dir = d0;
% % data.RT = d0;
% % data.iDir = d0;
% % data.iDirError = d0;
% % data.iEnd = d0;
% % 
% % data.targetAbs = targetAbs;
% % data.targetRel = targetRel;
% % data.start = start;
% % 
% % % rotate data into common coordinate frame - start at (0,0), target at
% % % (0,.12)
% % 
% % %%%% this is for 45 degree targets
% % % for j=1:data.Ntrials
% % %     data.Cr{j}=data.C{j};
% % % end
% % %%%%%
% % for j=1:data.Ntrials % iterate through all trials
% %     
% %     % if targets are on 45 degrees
% % %     if(data.targetRel(j,1)<0)
% % %         data.Cr{j}(:,1)=-data.Cr{j}(:,1);
% % %         data.pert(j)=-data.pert(j);
% % %     else
% % %         data.Cr{j}(:,1)=data.Cr{j}(:,1);
% % %     end
% % %     if(data.targetRel(j,2)<0)
% % %         data.Cr{j}(:,2)=-data.Cr{j}(:,2);
% % %         data.pert(j)=-data.pert(j);
% % %     else
% % %         data.Cr{j}(:,2)=data.Cr{j}(:,2);
% % %     end    
% %     
% %     % if targts are on x/y axis 
% %     theta(j) = atan2(data.targetRel(j,2),data.targetRel(j,1))-pi/2;
% %     R = [cos(theta(j)) sin(theta(j)); -sin(theta(j)) cos(theta(j))];
% %     
% %     data.Cr{j} = (R*(data.C{j}'-repmat(start(j,:),size(data.C{j},1),1)'))';
% %     %data.Nr{j} = (R*(data.N{j}'))';
% % end
% % %data.theta = theta;
% % 
