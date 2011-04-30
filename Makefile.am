## Process this file with automake to produce Makefile.in
SUBDIRS = src/

EXTRA_DIST = icons/button_f10.png \
	icons/button_f7.png \
	icons/button_f8.png \
	icons/button_f9.png \
	icons/cam_external.png \
	icons/cam_front.png \
	icons/cam_rear.png \
	icons/cpanel.png \
	icons/cpan_f2_map.png \
	icons/cpan_f2_normal.png \
	icons/object_brown_dwarf.png \
	icons/object_planet_dwarf.png \
	icons/object_planet_large_gas_giant.png \
	icons/object_planet_medium_gas_giant.png \
	icons/object_planet_small_gas_giant.png \
	icons/object_planet_small.png \
	icons/object_planet_life.png \
	icons/object_star_a.png \
	icons/object_star_b.png \
	icons/object_star_f.png \
	icons/object_star_g.png \
	icons/object_star_k.png \
	icons/object_star_m.png \
	icons/object_star_o.png \
	icons/sectorview_f6_systeminfo.png \
	icons/timeaccel0_on.png \
	icons/timeaccel0.png \
	icons/timeaccel1_on.png \
	icons/timeaccel1.png \
	icons/timeaccel2_on.png \
	icons/timeaccel2.png \
	icons/timeaccel3_on.png \
	icons/timeaccel3.png \
	icons/timeaccel4_on.png \
	icons/timeaccel4.png \
	icons/zoom_in_f7.png \
	icons/zoom_out_f8.png \
	icons/hyperspace_f8.png \
	icons/comms_f4.png \
	icons/sysview_accel_f1_on.png icons/sysview_accel_f1.png icons/sysview_accel_f2_on.png icons/sysview_accel_f2.png icons/sysview_accel_f3_on.png icons/sysview_accel_f3.png icons/sysview_accel_r1_on.png icons/sysview_accel_r1.png icons/sysview_accel_r2_on.png icons/sysview_accel_r2.png icons/sysview_accel_r3_on.png icons/sysview_accel_r3.png \
	msvc/pioneer-msvc-9.0.sln msvc/pioneer-msvc-9.0.vcproj \
	config.ini

.PHONY: codedoc
codedoc:
	@naturaldocs -i src/ -o HTML codedoc/ -p . -do
