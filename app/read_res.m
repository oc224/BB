#! /usr/bin/octave -qf

arg_list = argv();
filename=arg_list{1}
filename=[filename '.out'];
%function read_res(filename)
fd=fopen(filename);
data=fread(fd,"float32");
fclose(fd);

%data=reshape(data,2,length(data)/2)';
%data=data(:,1)+1i*data(:,2);
N=length(data);
t=(0:N-1)/10240*1000;

plot(t,data)
xlabel('msec');
title(filename)
keyboard
%end
