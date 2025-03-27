// Enclosure for ESP32FW / ULN2003 / 28BYJ-48 - Box

$fn=40;

// Electronics lid definitions (copy and paste from corresponding SCAD file)
box_lid_screw_hole_radius=1;

box_drv_support_diameter=6;
box_drv_support_length=4;
box_drv_support_hole_depth=3;
box_drv_length=29.6;
box_drv_width=26.9;
box_drv_side_offset=2.7+1;

box_esp32c3_back=23.4;
box_esp32c3_width=17.8;
box_esp32c3_depth=11;
box_esp32c3_board_1=5.5;
box_esp32c3_board_thickness=1.3;
box_esp32c3_board_groove=2;
box_esp32c3_board_extra=3;
box_esp32c3_block_wall=box_esp32c3_back-box_esp32c3_depth;

// Electronics box definitions
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
box_usb_length=8.8;
box_usb_height=3.2;
box_usb_offset=5.4;
box_usb_distance=0;

// Box
difference()
 {
  // Box attaches to the wheel and provides support for the lid which holds the motor and the electronics 
   cube([box_length,box_width,box_height]);
   union()
   {
    // Box inner space
    translate([box_wall_thickness,box_wall_thickness,box_wall_thickness])
     cube([box_length-2*box_wall_thickness,box_width-2*box_wall_thickness,box_height-2*box_wall_thickness]);
    // Space for lid
    translate([box_wall_thickness,0,box_height-box_wall_thickness])
     cube([box_length-2*box_wall_thickness,box_width,box_wall_thickness]);
    // Hole for filter wheel
    translate([(box_length-box_hole_length)/2,0,box_wall_thickness])
     cube([box_hole_length,box_wall_thickness,box_hole_height]);
    // Holes for filter wheel screws 
    translate([box_screw_1_x,box_screw_y,0])
     cylinder(box_wall_thickness,d=box_screw_diameter);
    translate([box_screw_2_x,box_screw_y,0])
     cylinder(box_wall_thickness,d=box_screw_diameter);
    // Hole for ESP32-C3 usb connector
   translate([box_length-box_boards_length-0*box_wall_thickness+box_lid_support_mid_radius+box_esp32c3_board_extra+box_esp32c3_width/2-box_usb_length/2,box_width-box_wall_thickness,box_height-box_wall_thickness-box_usb_offset-box_usb_height])   
     cube([box_usb_length,box_wall_thickness,box_usb_height]);
   }
 } 
 
// Lid supports
difference()
 {
 union()
  {
   // Corner supports
   box_corner(0,box_wall_thickness,box_wall_thickness,box_wall_thickness,box_height-2*box_wall_thickness);
   box_corner(-90,box_wall_thickness,box_width-box_wall_thickness,box_wall_thickness,box_height-2*box_wall_thickness);
   box_corner(180,box_length-box_wall_thickness,box_width-box_wall_thickness,box_wall_thickness,box_height-2*box_wall_thickness);
   box_corner(90,box_length-box_wall_thickness,box_wall_thickness,box_wall_thickness,box_height-2*box_wall_thickness);
   
   // Middle supports
   translate([box_length/2-box_lid_support_mid_radius/2,box_wall_thickness,box_hole_height+box_wall_thickness])
    cube([box_lid_support_mid_radius,box_lid_support_mid_radius,box_height-box_hole_height-2*box_wall_thickness]);
   translate([box_length-box_boards_length-box_wall_thickness-box_lid_support_mid_radius/2,box_width-box_wall_thickness-box_lid_support_mid_radius,box_wall_thickness])
     cube([box_lid_support_mid_radius,box_lid_support_mid_radius,box_height-2*box_wall_thickness]);
  }
 union()
  {
   // Corner support holes
   box_lid_support_hole(box_wall_thickness+box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_wall_thickness+box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_side_height-box_lid_support_hole_depth);
   box_lid_support_hole(box_wall_thickness+box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_width-box_wall_thickness-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_side_height-box_lid_support_hole_depth);
   box_lid_support_hole(box_length-box_wall_thickness-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_wall_thickness+box_corner_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_side_height-box_lid_support_hole_depth);
   box_lid_support_hole(box_length-box_wall_thickness-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_width-box_wall_thickness-box_corner_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_side_height-box_lid_support_hole_depth);

   // Middle supports holes   
   box_lid_support_hole(box_length/2,box_wall_thickness+box_lid_support_mid_radius-box_lid_support_hole_offset-box_lid_support_hole_radius,box_side_height-box_lid_support_hole_depth);
   box_lid_support_hole(box_length-box_boards_length-box_wall_thickness,box_width-box_wall_thickness-box_lid_support_mid_radius+box_lid_support_hole_offset+box_lid_support_hole_radius,box_side_height-box_lid_support_hole_depth);
      
  }
 }

// Modules

module box_corner(angle,x,y,z,length)
 {
  translate([x,y,z])
   rotate([0,0,angle])
    intersection()
     {
//      cylinder(h=scale*(box_side_height)-box_lid_thickness,r=box_corner_radius,center=false);
      cube(size=[box_corner_radius,box_corner_radius,length]);
     }      
 }
 
module box_lid_support_hole(x,y,z)
 {
  translate([x,y,z])
   cylinder(h=box_lid_support_hole_depth,r=box_lid_support_hole_radius);
 }
