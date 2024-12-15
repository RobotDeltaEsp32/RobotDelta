clc;
clear;
close all;

vertices = [
    -50,  0, -80;
    -25,-40, -80;
     25,-40, -80;
     50,  0, -80;
     25, 40, -80;
    -25, 40, -80;
    -50,  0, -110;
    -25,-40, -110;
     25,-40, -110;
     50,  0, -110;
     25, 40, -110;
    -25, 40, -110;
];

faces = [
    1,2,3,4,5,6;
    7,8,9,10,11,12;
];
faces2 = [
    1,2,8,7;
    2,3,9,8;
    3,4,10,9;
    4,5,11,10;
    5,6,12,11;
    6,1,7,12;
    ];

figure;
hold on;

patch('Vertices', vertices, 'Faces', faces, 'Faces', faces2, ...
    'FaceColor', 'green', 'FaceAlpha', 0.3, ... 
    'EdgeColor', 'black');

for i = 1:size(vertices, 1)
    text(vertices(i, 1), vertices(i, 2), vertices(i, 3), ...
        sprintf('%d, %d, %d', vertices(i, :)), ... 
        'FontSize', 10, 'Color', 'black', 'HorizontalAlignment', 'center');
end

xlabel('X (mm)');
ylabel('Y (mm)');
zlabel('Z (mm)');
title('Figura del Prisma Rectangular');
grid on;
axis equal;
view(3);

R = 60;  
r = 35;
L1 = 60; 
L2 = 90;  

x_range = -250:5:250; 
y_range = -200:5:200; 
z_range = 0:-5:-150; 

workspace_points = [];
[X, Y, Z] = meshgrid(x_range, y_range, z_range);
points = [X(:), Y(:), Z(:)];

for i = 1:size(points, 1)
    point = points(i, :); 
    [q, is_reachable] = inverse_kinematics(point, R, r, L1, L2);
    if is_reachable
        workspace_points = [workspace_points; point];
    end
end

scatter3(workspace_points(:, 1), workspace_points(:, 2), workspace_points(:, 3), 10, 'b', 'filled');
xlabel('X (mm)');
ylabel('Y (mm)');
zlabel('Z (mm)');
title('Workspace del Robot Delta');
grid on;
axis equal;

function [q, is_reachable] = inverse_kinematics(point, R, r, L1, L2)
    x = point(1);  
    y = point(2);  
    z = point(3); 

    a1 = -2*z*L1;
    b1 = 2*L1*(x*cosd(0)+y*sind(0)+r-R);
    c1 = (x+cosd(0)*(r-R))^2+(y+sind(0)*(r-R))^2+(z^2)+(L1^2)-(L2^2);

    a2 = -2*z*L1;
    b2 = 2*L1*(x*cosd(120)+y*sind(120)+r-R);
    c2 = (x+cosd(120)*(r-R))^2+(y+sind(120)*(r-R))^2+(z^2)+(L1^2)-(L2^2);

    a3 = -2*z*L1;
    b3 = 2*L1*(x*cosd(240)+y*sind(240)+r-R);
    c3 = (x+cosd(240)*(r-R))^2+(y+sind(240)*(r-R))^2+(z^2)+(L1^2)-(L2^2);

    try
        q1 = asind(c1/sqrt((a1^2)+(b1^2)))-atand(b1/a1); 
        q2 = asind(c2/sqrt((a2^2)+(b2^2)))-atand(b2/a2);
        q3 = asind(c3/sqrt((a3^2)+(b3^2)))-atand(b3/a3);

        if all(q1>0 & q1<90) && ...
           all(q2>0 & q2<90) && ...
           all(q3>0 & q3<90)
            is_reachable = true;
        else
            is_reachable = false;
        end
        q = [q1, q2, q3];
    catch
        q = [];
        is_reachable = false;
    end
end