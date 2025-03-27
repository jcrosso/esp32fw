// Enclosure for ESP32FW / ULN2003 / 28BYJ-48 - Lid

$fn=40;

// Electronics box definitions (copy and paste from corresponding scad file)
box_length=124;
box_width=50;
box_height=50;
box_lid_thickness=2;
box_wall_thickness=2;
box_hole_height=22.5*0+23.5;
box_hole_length=108;
box_screw_separation=70;
box_screw_1_x=25.5;
box_screw_2_x=box_screw_1_x+box_screw_separation;
box_screw_y=24;
box_screw_diameter=3.2;
box_lid_support_hole_depth=3.5;
box_lid_support_hole_radius=3.1/2;
box_lid_support_hole_offset=1.5;
box_lid_support_mid_radius=2*box_lid_support_hole_radius+2*box_lid_support_hole_offset;
box_corner_radius=5;
box_boards_length=70;
box_side_height=box_height-box_lid_thickness;

// Lid specific definitions

// Do not copy and paste these definitions in the box scad
box_lid_length=box_length-2*box_wall_thickness;
box_lid_width=box_width;
box_width_actual=box_width-2*box_wall_thickness;
box_drv_support_hole_diameter=2*box_lid_support_hole_radius;
motor_support_diameter=6.5;
motor_support_length=19.0;
motor_support_hole_radius=3.6/2;
motor_support_hole_depth=7;
motor_support_1_x=8+4.5+box_wall_thickness;
motor_support_1_y=box_width-8;
motor_support_2_x=27.6+4.5+box_wall_thickness;
motor_support_2_y=13;
motor_support_skirt_height=6;
motor_support_skirt_radius_1=8-box_wall_thickness;
motor_support_skirt_radius_2=motor_support_diameter/2;

// These should be copied
box_lid_screw_hole_radius=1;

box_drv_support_diameter=6;
box_drv_support_length=4;
box_drv_support_hole_depth=3;
box_drv_length=29.6;
box_drv_width=26.9;
box_drv_side_offset=2.7+1;

box_esp32c3_back=23.6+box_lid_thickness;
box_esp32c3_width=17.8;
box_esp32c3_depth=11;
box_esp32c3_board_1=5.5;
box_esp32c3_board_thickness=1.3;
box_esp32c3_board_groove=2;
box_esp32c3_board_extra=3;
box_esp32c3_block_wall=box_esp32c3_back-box_esp32c3_depth;

// ESP32-C3 support
difference()
 {
   union()
    {
     translate([box_length-box_boards_length-box_wall_thickness+box_lid_support_mid_radius,box_width-box_esp32c3_depth-box_esp32c3_block_wall,-(box_esp32c3_board_1+box_esp32c3_board_extra)])
      cube([box_esp32c3_board_extra+box_esp32c3_board_groove,box_esp32c3_depth,box_esp32c3_board_1+box_esp32c3_board_extra]);
     translate([box_length-box_boards_length-box_wall_thickness+box_lid_support_mid_radius+(box_esp32c3_board_extra+box_esp32c3_width-box_esp32c3_board_groove),box_width-(box_esp32c3_depth)-box_esp32c3_block_wall,-(box_esp32c3_board_1+box_esp32c3_board_extra)])
      cube([box_esp32c3_board_extra+box_esp32c3_board_groove,box_esp32c3_depth,box_esp32c3_board_1+box_esp32c3_board_extra]);
     translate([box_length-box_boards_length-box_wall_thickness+box_lid_support_mid_radius,box_width-box_esp32c3_depth-box_esp32c3_block_wall-box_esp32c3_board_extra,-(box_esp32c3_board_1+box_esp32c3_board_extra)])
      cube([2*box_esp32c3_board_extra+box_esp32c3_width,box_esp32c3_board_extra,box_esp32c3_board_1+box_esp32c3_board_extra]);
    }
   translate([box_length-box_boards_length-box_wall_thickness+box_lid_support_mid_radius+box_esp32c3_board_extra,box_width-box_esp32c3_depth-box_esp32c3_block_wall,-box_esp32c3_board_1])
      cube([box_esp32c3_width,box_esp32c3_depth,box_esp32c3_board_thickness]); 
 }

// Lid
difference()
 {
  // Base   
  cube([box_lid_length,box_width,box_wall_thickness]);  
  
  // Screw holes
   
  // Corner support holes  
  box_lid_hole(box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_wall_thickness+box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius);
  box_lid_hole(box_lid_length-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_wall_thickness+box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius);     
  box_lid_hole(box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_lid_width-box_wall_thickness-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius);
  box_lid_hole(box_lid_length-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_lid_width-box_wall_thickness-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius);
     
  // Mid support holes
  box_lid_hole(box_length/2-box_wall_thickness,box_wall_thickness+box_lid_support_mid_radius-box_lid_support_hole_offset-box_lid_support_hole_radius);
  box_lid_hole(box_length-box_boards_length-2*box_wall_thickness,box_width-box_wall_thickness-box_lid_support_mid_radius+box_lid_support_hole_offset+box_lid_support_hole_radius);
 }

// Motor support
 
// Main supports 
translate([motor_support_1_x,motor_support_1_y,-motor_support_length])
 difference()
  {
   cylinder(motor_support_length,d=motor_support_diameter);
   cylinder(motor_support_hole_depth,r=motor_support_hole_radius);
  }
translate([motor_support_2_x,motor_support_2_y,-motor_support_length])
 difference()
  {
   cylinder(motor_support_length,d=motor_support_diameter);
   cylinder(motor_support_hole_depth,r=motor_support_hole_radius);
  }  

// Skirts
difference()
 {
  union()
   {
    translate([motor_support_1_x,motor_support_1_y,-motor_support_skirt_height])
     cylinder(h=motor_support_skirt_height,r1=motor_support_skirt_radius_2,r2=motor_support_skirt_radius_1);
    translate([motor_support_2_x,motor_support_2_y,-motor_support_skirt_height])
     cylinder(h=motor_support_skirt_height,r1=motor_support_skirt_radius_2,r2=motor_support_skirt_radius_1);
   }
   translate([motor_support_1_x+(motor_support_2_x-motor_support_1_x)/2,motor_support_1_y+(motor_support_2_y-motor_support_1_y)/2,-motor_support_skirt_height])
   cylinder(h=motor_support_skirt_height,r=sqrt((motor_support_2_x-motor_support_1_x)^2+(motor_support_2_y-motor_support_1_y)^2)/2-motor_support_diameter/2);
 }
 
// Motor driver board supports
box_drv_support(box_lid_length-box_drv_side_offset,box_lid_width-box_wall_thickness-box_drv_side_offset-box_corner_radius);
box_drv_support(box_lid_length-box_drv_side_offset,box_lid_width-box_wall_thickness-box_drv_side_offset-box_corner_radius-box_drv_length); 
box_drv_support(box_lid_length-box_drv_side_offset-box_drv_width,box_lid_width-box_wall_thickness-box_drv_side_offset-box_corner_radius);
box_drv_support(box_lid_length-box_drv_side_offset-box_drv_width,box_lid_width-box_wall_thickness-box_drv_side_offset-box_corner_radius-box_drv_length); 
 
// Modules
module box_lid_hole(x,y)
 {
  translate([x,y,0])
   cylinder(h=box_wall_thickness,r=box_lid_screw_hole_radius);
 }

module box_drv_support(x,y)
 {
  translate([x,y,-box_drv_support_length])
   difference()
    {
    cylinder(h=box_drv_support_length,d=box_drv_support_diameter);
    cylinder(h=box_drv_support_hole_depth,d=box_drv_support_hole_diameter);
    }    
 }   
 
 
/*
    // Driver board support
    box_drv_support(-box_drv_x,box_width/2+box_drv_side/2,box_wall_thickness);
    box_drv_support(-box_drv_x,box_width/2-box_drv_side/2,box_wall_thickness);
    box_drv_support(-box_drv_x-box_drv_side,box_width/2+box_drv_side/2,box_wall_thickness);
    box_drv_support(-box_drv_x-box_drv_side,box_width/2-box_drv_side/2,box_wall_thickness);
     // Driver board screw holes in supports
    box_drv_hole(-box_drv_x,box_width/2+box_drv_side/2,box_wall_thickness);
    box_drv_hole(-box_drv_x,box_width/2-box_drv_side/2,box_wall_thickness);
    box_drv_hole(-box_drv_x-box_drv_side,box_width/2+box_drv_side/2,box_wall_thickness);
    box_drv_hole(-box_drv_x-box_drv_side,box_width/2-box_drv_side/2,box_wall_thickness);   
    
   // ESP board support
    box_esp32_support(-box_length,box_width_actual/2+box_esp32_length/2+box_wall_thickness-box_esp32_support_side,box_wall_thickness);
    box_esp32_support(-box_length,box_width_actual/2-box_esp32_length/2+box_wall_thickness,box_wall_thickness);
    box_esp32_support(-box_length+box_esp32_width,box_width_actual/2+box_esp32_length/2+box_wall_thickness-box_esp32_support_side,box_wall_thickness);
    box_esp32_support(-box_length+box_esp32_width,box_width_actual/2-box_esp32_length/2+box_wall_thickness,box_wall_thickness);

    // ESP32 clips
    box_esp32_clip(-box_length+box_esp32_width+box_esp32_support_side,box_wall_thickness,box_wall_thickness);
    box_esp32_clip(-box_length+box_esp32_width+box_esp32_support_side,box_width_actual+box_wall_thickness-box_esp32_clip_length,box_wall_thickness);  
    
        // ESP vents
    box_esp32_vent(-box_length+box_esp32_width*0.2,box_width_actual/2-box_esp32_vent_length/2+box_wall_thickness,0);   
    box_esp32_vent(-box_length+box_esp32_width*0.5,box_width_actual/2-box_esp32_vent_length/2+box_wall_thickness,0);
    box_esp32_vent(-box_length+box_esp32_width*0.8,box_width_actual/2-box_esp32_vent_length/2+box_wall_thickness,0);


module box_drv_support(x,y,z)
 {
  translate([x,y,z])
   cylinder(h=box_drv_height,r=box_drv_radius,center=false);
 }

module box_drv_hole(x,y,z)
 {
  translate([x,y,z])
   cylinder(h=box_drv_height,r=box_drv_hole_radius,center=false);
 }

module box_esp32_support(x,y,z)
 {
  translate([x,y,z])
   cube([box_esp32_support_side,box_esp32_support_side,box_esp32_support_height]);
 }
 
module box_esp32_clip(x,y,z)
 {
  translate([x,y,z])
   cube([box_esp32_clip_thickness,box_esp32_clip_length,box_esp32_clip_height]);
 }
 
module box_esp32_vent(x,y,z)
 {
  translate([x,y,z])
   cube([box_esp32_vent_width,box_esp32_vent_length,box_wall_thickness]);
 } 

 */  
    