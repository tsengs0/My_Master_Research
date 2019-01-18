M1=csvread('Total_csv_90/Total_NonDVFS.0.csv');
M2=csvread('Total_csv_90/Total_DVFS.NonOverhead.0.csv');
M3=csvread('Total_csv_90/Total_DVFS.Overhead.0.csv');

M4=csvread('Total_csv_90/Total_NonDVFS.1.csv');
M5=csvread('Total_csv_90/Total_DVFS.NonOverhead.1.csv');
M6=csvread('Total_csv_90/Total_DVFS.Overhead.1.csv');

M7=csvread('Total_csv_90/Total_NonDVFS.2.csv');
M8=csvread('Total_csv_90/Total_DVFS.NonOverhead.2.csv');
M9=csvread('Total_csv_90/Total_DVFS.Overhead.2.csv');

M10=csvread('Total_csv_90/Total_NonDVFS.3.csv');
M11=csvread('Total_csv_90/Total_DVFS.NonOverhead.3.csv');
M12=csvread('Total_csv_90/Total_DVFS.Overhead.3.csv');

M13=csvread('Total_csv_90/Total_NonDVFS.4.csv');
M14=csvread('Total_csv_90/Total_DVFS.NonOverhead.4.csv');
M15=csvread('Total_csv_90/Total_DVFS.Overhead.4.csv');

figure(1);
x_axis = M1(:, 1);
RFJ_Env1 = M1(:, 2);
RFJ_Env2 = M2(:, 2);
RFJ_Env3 = M3(:, 2);
AFJ_Env1 = M1(:, 3);
AFJ_Env2 = M2(:, 3);
AFJ_Env3 = M3(:, 3);
TargetResponse_Env1 = M1(:, 4);
TargetResponse_Env2 = M2(:, 4);
TargetResponse_Env3 = M3(:, 4);
AverageResponse_Env1 = M1(:, 5);
AverageResponse_Env2 = M2(:, 5);
AverageResponse_Env3 = M3(:, 5);
Energy_Env1 = M1(:, 6);
Energy_Env2 = M2(:, 6);
Energy_Env3 = M3(:, 6);
subplot(2,2,1);
plot(x_axis, RFJ_Env1, x_axis, RFJ_Env2, x_axis, RFJ_Env3, x_axis, RFJ_Env4, x_axis, RFJ_Env5);
xlabel("Target Response Time(%)");
ylabel("Relative Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,2);
plot(x_axis, AFJ_Env1, x_axis, AFJ_Env2, x_axis, AFJ_Env3);
xlabel("Target Response Time(%)");
ylabel("Absolute Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,3);
plot(TargetResponse_Env1, AverageResponse_Env1, TargetResponse_Env2, AverageResponse_Env2, TargetResponse_Env3, AverageResponse_Env3);
xlabel("Target Response Time(us)");
ylabel("Average Response Time(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,4);
plot(x_axis, Energy_Env1, x_axis, Energy_Env2, x_axis, Energy_Env3);
xlabel("Target Response Time(%)");
ylabel("Energy Consumption(uJ)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});

figure(2);
x_axis_Tsk1 = M4(:, 1);
RFJ_Env1_Tsk1 = M4(:, 2);
RFJ_Env2_Tsk1 = M5(:, 2);
RFJ_Env3_Tsk1 = M6(:, 2);
AFJ_Env1_Tsk1 = M4(:, 3);
AFJ_Env2_Tsk1 = M5(:, 3);
AFJ_Env3_Tsk1 = M6(:, 3);
TargetResponse_Env1_Tsk1 = M4(:, 4);
TargetResponse_Env2_Tsk1 = M5(:, 4);
TargetResponse_Env3_Tsk1 = M6(:, 4);
AverageResponse_Env1_Tsk1 = M4(:, 5);
AverageResponse_Env2_Tsk1 = M5(:, 5);
AverageResponse_Env3_Tsk1 = M6(:, 5);
Energy_Env1_Tsk1 = M4(:, 6);
Energy_Env2_Tsk1 = M5(:, 6);
Energy_Env3_Tsk1 = M6(:, 6);
subplot(2,2,1);
plot(x_axis_Tsk1, RFJ_Env1_Tsk1, x_axis_Tsk1, RFJ_Env2_Tsk1, x_axis_Tsk1, RFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Relative Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,2);
plot(x_axis_Tsk1, AFJ_Env1_Tsk1, x_axis_Tsk1, AFJ_Env2_Tsk1, x_axis_Tsk1, AFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Absolute Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,3);
plot(TargetResponse_Env1_Tsk1, AverageResponse_Env1_Tsk1, TargetResponse_Env2_Tsk1, AverageResponse_Env2_Tsk1, TargetResponse_Env3_Tsk1, AverageResponse_Env3_Tsk1);
xlabel("Target Response Time(us)");
ylabel("Average Response Time(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,4);
plot(x_axis_Tsk1, Energy_Env1_Tsk1, x_axis_Tsk1, Energy_Env2_Tsk1, x_axis_Tsk1, Energy_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Energy Consumption(uJ)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});

figure(3);
x_axis_Tsk1 = M4(:, 1);
RFJ_Env1_Tsk1 = M7(:, 2);
RFJ_Env2_Tsk1 = M8(:, 2);
RFJ_Env3_Tsk1 = M9(:, 2);
AFJ_Env1_Tsk1 = M7(:, 3);
AFJ_Env2_Tsk1 = M8(:, 3);
AFJ_Env3_Tsk1 = M9(:, 3);
TargetResponse_Env1_Tsk1 = M7(:, 4);
TargetResponse_Env2_Tsk1 = M8(:, 4);
TargetResponse_Env3_Tsk1 = M9(:, 4);
AverageResponse_Env1_Tsk1 = M7(:, 5);
AverageResponse_Env2_Tsk1 = M8(:, 5);
AverageResponse_Env3_Tsk1 = M9(:, 5);
Energy_Env1_Tsk1 = M7(:, 6);
Energy_Env2_Tsk1 = M8(:, 6);
Energy_Env3_Tsk1 = M9(:, 6);
subplot(2,2,1);
plot(x_axis_Tsk1, RFJ_Env1_Tsk1, x_axis_Tsk1, RFJ_Env2_Tsk1, x_axis_Tsk1, RFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Relative Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,2);
plot(x_axis_Tsk1, AFJ_Env1_Tsk1, x_axis_Tsk1, AFJ_Env2_Tsk1, x_axis_Tsk1, AFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Absolute Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,3);
plot(TargetResponse_Env1_Tsk1, AverageResponse_Env1_Tsk1, TargetResponse_Env2_Tsk1, AverageResponse_Env2_Tsk1, TargetResponse_Env3_Tsk1, AverageResponse_Env3_Tsk1);
xlabel("Target Response Time(us)");
ylabel("Average Response Time(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,4);
plot(x_axis_Tsk1, Energy_Env1_Tsk1, x_axis_Tsk1, Energy_Env2_Tsk1, x_axis_Tsk1, Energy_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Energy Consumption(uJ)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});

figure(4);
x_axis_Tsk1 = M4(:, 1);
RFJ_Env1_Tsk1 = M10(:, 2);
RFJ_Env2_Tsk1 = M11(:, 2);
RFJ_Env3_Tsk1 = M12(:, 2);
AFJ_Env1_Tsk1 = M10(:, 3);
AFJ_Env2_Tsk1 = M11(:, 3);
AFJ_Env3_Tsk1 = M12(:, 3);
TargetResponse_Env1_Tsk1 = M10(:, 4);
TargetResponse_Env2_Tsk1 = M11(:, 4);
TargetResponse_Env3_Tsk1 = M12(:, 4);
AverageResponse_Env1_Tsk1 = M10(:, 5);
AverageResponse_Env2_Tsk1 = M11(:, 5);
AverageResponse_Env3_Tsk1 = M12(:, 5);
Energy_Env1_Tsk1 = M10(:, 6);
Energy_Env2_Tsk1 = M11(:, 6);
Energy_Env3_Tsk1 = M12(:, 6);
subplot(2,2,1);
plot(x_axis_Tsk1, RFJ_Env1_Tsk1, x_axis_Tsk1, RFJ_Env2_Tsk1, x_axis_Tsk1, RFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Relative Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,2);
plot(x_axis_Tsk1, AFJ_Env1_Tsk1, x_axis_Tsk1, AFJ_Env2_Tsk1, x_axis_Tsk1, AFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Absolute Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,3);
plot(TargetResponse_Env1_Tsk1, AverageResponse_Env1_Tsk1, TargetResponse_Env2_Tsk1, AverageResponse_Env2_Tsk1, TargetResponse_Env3_Tsk1, AverageResponse_Env3_Tsk1);
xlabel("Target Response Time(us)");
ylabel("Average Response Time(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,4);
plot(x_axis_Tsk1, Energy_Env1_Tsk1, x_axis_Tsk1, Energy_Env2_Tsk1, x_axis_Tsk1, Energy_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Energy Consumption(uJ)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});

figure(5);
x_axis_Tsk1 = M4(:, 1);
RFJ_Env1_Tsk1 = M13(:, 2);
RFJ_Env2_Tsk1 = M14(:, 2);
RFJ_Env3_Tsk1 = M15(:, 2);
AFJ_Env1_Tsk1 = M13(:, 3);
AFJ_Env2_Tsk1 = M13(:, 3);
AFJ_Env3_Tsk1 = M14(:, 3);
TargetResponse_Env1_Tsk1 = M13(:, 4);
TargetResponse_Env2_Tsk1 = M14(:, 4);
TargetResponse_Env3_Tsk1 = M15(:, 4);
AverageResponse_Env1_Tsk1 = M13(:, 5);
AverageResponse_Env2_Tsk1 = M14(:, 5);
AverageResponse_Env3_Tsk1 = M15(:, 5);
Energy_Env1_Tsk1 = M13(:, 6);
Energy_Env2_Tsk1 = M14(:, 6);
Energy_Env3_Tsk1 = M15(:, 6);
subplot(2,2,1);
plot(x_axis_Tsk1, RFJ_Env1_Tsk1, x_axis_Tsk1, RFJ_Env2_Tsk1, x_axis_Tsk1, RFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Relative Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,2);
plot(x_axis_Tsk1, AFJ_Env1_Tsk1, x_axis_Tsk1, AFJ_Env2_Tsk1, x_axis_Tsk1, AFJ_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Absolute Finish Time Jitter(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,3);
plot(TargetResponse_Env1_Tsk1, AverageResponse_Env1_Tsk1, TargetResponse_Env2_Tsk1, AverageResponse_Env2_Tsk1, TargetResponse_Env3_Tsk1, AverageResponse_Env3_Tsk1);
xlabel("Target Response Time(us)");
ylabel("Average Response Time(us)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});
subplot(2,2,4);
plot(x_axis_Tsk1, Energy_Env1_Tsk1, x_axis_Tsk1, Energy_Env2_Tsk1, x_axis_Tsk1, Energy_Env3_Tsk1);
xlabel("Target Response Time(%)");
ylabel("Energy Consumption(uJ)");
legend({'NonDVFS','User-Specified DVFS','Profile-based DVFS'});




