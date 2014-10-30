#! /usr/bin/octave -qf

arg_list = argv();
mode=length(arg_list)

filename=arg_list{1}

switch (mode)
case 1
	filename=[filename '.out'];
	fd=fopen(filename);
	data=fread(fd,"float32");
	fclose(fd);
	N=length(data);
	t=(0:N-1)/10240*1000;

	%plot
	plot(t,data)
	xlabel('msec');
	title(filename)
	keyboard

case 2
	pkg load signal
	rxname=[filename '.wav']
	txname=arg_list{2}

	rx=wavread(rxname);
	tx=wavread(txname);
	rx=complex(rx(:,1),rx(:,2));
	tx=complex(tx(:,1),tx(:,2));
	[data,lags]=xcorr(rx,tx);
	data_phase=angle(data);
	data=abs(data);

	%plot
	subplot(2,1,1)
	plot(lags/10240*1000,data)
	xlabel('msec');
	subplot(2,1,2)
	plot(lags/10240*1000,data_phase);
	xlabel('msec');
	title(filename)
	keyboard

endswitch






%data=reshape(data,2,length(data)/2)';
%data=data(:,1)+1i*data(:,2);


