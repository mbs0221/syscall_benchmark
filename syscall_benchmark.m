clear variables;
syscalls = categorical({'nothing', 'getpid', 'getuid', 'time', 'read', 'ioctl', 'add\_key', 'get\_mempolicy', 'clock\_gettime', 'mincore'});

nothing = importdata("nothing.txt");
% Excluding the outliers from the average
nothing = nothing(nothing < 4000);
nothing_mean = round(mean(nothing));
means(1) = nothing_mean;

getpid = importdata("getpid.txt");
getpid = getpid(getpid < 4000);
getpid_mean = round(mean(getpid));
means(2) = getpid_mean;

getuid = importdata("getuid.txt");
getuid = getuid(getuid < 4000);
getuid_mean = round(mean(getuid));
means(3) = getuid_mean;

time = importdata("time.txt");
time = time(time < 4000);
time_mean = round(mean(time));
means(4) = time_mean;

read = importdata("read.txt");
read = read(read < 4000);
read_mean = round(mean(read));
means(5) = read_mean;

ioctl = importdata("ioctl.txt");
ioctl = ioctl(ioctl < 4000);
ioctl_mean = round(mean(ioctl));
means(6) = ioctl_mean;

add_key = importdata("add_key.txt");
add_key = add_key(add_key < 4000);
add_key_mean = round(mean(add_key));
means(7) = add_key_mean;

get_mempolicy = importdata("get_mempolicy.txt");
get_mempolicy = get_mempolicy(get_mempolicy < 4000);
get_mempolicy_mean = round(mean(get_mempolicy));
means(8) = get_mempolicy_mean;

clock_gettime = importdata("clock_gettime.txt");
clock_gettime = clock_gettime(clock_gettime < 4000);
clock_gettime_mean = round(mean(clock_gettime));
means(9) = clock_gettime_mean;

mincore = importdata("mincore.txt");
mincore = mincore(mincore < 4000);
mincore_mean = round(mean(mincore));
means(10) = mincore_mean;

bar(syscalls, means);
set(gca,'fontsize',32)
xlabel('System Call')
ylabel('CPU Cycles')