
fd=fopen('./13141938.out')
data=fread(fd,"float32");
fclose(fd)

%data=reshape(data,2,length(data)/2)';
%data=data(:,1)+1i*data(:,2);
N=length(data);
t=(0:N-1)/10240;

plot(t,data)
