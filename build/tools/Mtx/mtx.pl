#!/usr/bin/perl


use strict;
use warnings;
use Cwd;
my $cwd = getcwd();
chomp($cwd);
my $task_dir_pro = $ARGV[0];
my $task_dir_Pla = $ARGV[1];
my $CUSTOM_MEMORYDEVICE_H_NAME  = "$cwd/mediatek/custom/$task_dir_pro/preloader/inc/custom_MemoryDevice.h";
print "custom_MemoryDevice.h path:$CUSTOM_MEMORYDEVICE_H_NAME\n";
my $book;
my %REGION_TABLE;
my $BOOT1;
my $BOOT2;
my $RPMB;
my $USER;
my $emmc_sheet;
my $region_name;
my $region = 0;
my $boot1 = 2;
my $boot2 = 3;
my $rpmb = 4;
my $user = 9;


#push(@INC, "$cwd/mediatek/build/tools/Mtx/Spreadsheet");
push(@INC, 'mediatek/build/tools/Spreadsheet');
push(@INC, 'mediatek/build/tools');
require 'ParseExcel.pm';

my $temp_plat=substr($task_dir_Pla,2);

my $memory_device_list_name="MemoryDeviceList_MT$temp_plat.xls";
#print "start parse $task_dir\/$memory_device_list_name\n";
#lenovo-sw jixj 2014.2.12 modify
#my $ett_calc_table_excel= "$cwd/mediatek/build/tools/emigen/$task_dir_Pla/$memory_device_list_name";
my $ett_calc_table_excel= "$cwd/mediatek/build/tools/Mtx/emmc/$memory_device_list_name";

#lenovo-sw jixj 2013.11.6 add begin
if (-e "$cwd/mediatek/build/tools/Mtx/emmc/MemoryDeviceList_$ENV{PROJECT}.xls")
{
$ett_calc_table_excel = "$cwd/mediatek/build/tools/Mtx/emmc/MemoryDeviceList_$ENV{PROJECT}.xls";
}
#lenovo-sw jixj 2013.11.6 add end

print "memory list path $ett_calc_table_excel\n:";
my $EMMC_REGION_SHEET_NAME = "emmc_region";
my $EMMC_RegionBook = Spreadsheet::ParseExcel->new()->Parse($ett_calc_table_excel);


$emmc_sheet = get_sheet($EMMC_REGION_SHEET_NAME,$EMMC_RegionBook) ;
        unless ($emmc_sheet)
	{
		my $error_msg="Ptgen CAN NOT find sheet=$EMMC_REGION_SHEET_NAME in $ett_calc_table_excel\n";
		print $error_msg;
		die $error_msg;
	}
print "emmc sheet is:$emmc_sheet\n";
my $row=1;
$region_name = &xls_cell_value($emmc_sheet, $row, $region,$EMMC_REGION_SHEET_NAME);
print "region name is $region_name\n";
while($region_name ne "END"){
		$region_name	=~ s/\s+//g;
		$BOOT1     = &xls_cell_value($emmc_sheet, $row, $boot1,$EMMC_REGION_SHEET_NAME);
                #print "$region_name $BOOT1 size:\n";
		$BOOT2     = &xls_cell_value($emmc_sheet, $row, $boot2,$EMMC_REGION_SHEET_NAME);
		$RPMB   = &xls_cell_value($emmc_sheet, $row, $rpmb,$EMMC_REGION_SHEET_NAME);
		$USER	= &xls_cell_value($emmc_sheet, $row, $user,$EMMC_REGION_SHEET_NAME);
		$REGION_TABLE{$region_name}	= {BOOT1=>$BOOT1,BOOT2=>$BOOT2,RPMB=>$RPMB,USER=>$USER};
		#print "In $region_name,$BOOT1,$BOOT2,$RPMB,$USER\n";
		$row++;
		$region_name = &xls_cell_value($emmc_sheet, $row, $region,$EMMC_REGION_SHEET_NAME);
	}
open (CUSTOM_MEMORYDEVICE_H_NAME, "<$CUSTOM_MEMORYDEVICE_H_NAME") or &error_handler("mtxgen open CUSTOM_MEMORYDEVICE_H_NAME fail!\n", __FILE__, __LINE__);
        my @lines;
	my $iter = 0;
	my $part_num;	
	my $MAX_address = 0;
	my $combo_start_address = 0;
	my $cur=0;
	my $cur_user=0;
while (<CUSTOM_MEMORYDEVICE_H_NAME>) {
		my($line) = $_;
  		chomp($line);
		if ($line =~ /^#define\sCS_PART_NUMBER\[[0-9]\]/) {
#			print "$'\n";
			$lines[$iter] = $';
			$lines[$iter] =~ s/\s+//g;
			if ($lines[$iter] =~ /(.*)\/\/(.*)/) {
				$lines[$iter] =$1;
			}
			#print "$lines[$iter] \n";
			$iter ++;
		}
			
	}
my $configfile;
my $config_dir="$cwd/out/target/product/$task_dir_pro/MTXGen/config/";
#lenovo-sw jixj 2013.10.23 add begin
my $mtx_dir="$cwd/out/target/product/$task_dir_pro/MTXGen/";
mkdir ($mtx_dir)if (!-d $mtx_dir);
#lenovo-sw jixj 2013.10.23 add end
mkdir ($config_dir)if (!-d $config_dir);
#lenovo-sw jixj 2013.10.23 add begin
if(system ("cp $cwd/mediatek/build/tools/Mtx/MTXGenExe $cwd/out/target/product/$task_dir_pro/MTXGen/MTXGenExe") == 0)
{
  print "mtxgen cp MTXGenExe file OK!\n";
  
}
else
{
   print "mtxgen cp config fileMTXGenExeFail!\n";
   die;
}

if(system ("cp -r $cwd/mediatek/build/tools/Mtx/config $cwd/out/target/product/$task_dir_pro/MTXGen/")== 0)
{
  print "mtxgen cp config file OK!\n";
  
}
else
{
   print "mtxgen cp config file Fail!\n";
   die;
}
#lenovo-sw jixj 2013.10.23 add end
$configfile = $config_dir ."emmc.layout.xml";
print "config = $configfile\n" ;
 #`chmod 777 $config_dir` if (-e $config_dir);
open (CONFIGFILE, ">$configfile") or &error_handler("mtxgen open CONFIGFILE fail!\n", __FILE__, __LINE__);
print CONFIGFILE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<emmc-info>\n";
foreach $part_num (@lines){
     if(exists $REGION_TABLE{$part_num}){
          $cur = $REGION_TABLE{$part_num}{BOOT1} + $REGION_TABLE{$part_num}{BOOT2} + $REGION_TABLE{$part_num}{RPMB};
	  $cur_user = $REGION_TABLE{$part_num}{USER};
	  #print "Chose region layout: $part_num, $REGION_TABLE{$part_num}{BOOT1} + $REGION_TABLE{$part_num}{BOOT2} + $REGION_TABLE{$part_num}{RPMB}=$cur \$REGION_TABLE{\$part_num}{USER}=$cur_user\n";
          my $x_boot1=sprintf ("%x",$REGION_TABLE{$part_num}{BOOT1}*1024);
          my $x_boot2=sprintf ("%x",$REGION_TABLE{$part_num}{BOOT2}*1024);
          my $x_rpmb=sprintf ("%x",$REGION_TABLE{$part_num}{RPMB}*1024);
          my $x_user=sprintf ("%x",$REGION_TABLE{$part_num}{USER}*1024);
         
          #my $t_sheet=substr($task_dir,2);
          my $platsheet="mt$temp_plat";
         
          my $plat_sheet = get_sheet($platsheet,$EMMC_RegionBook);
          #print "erjng:$plat_sheet\n";
          my $emmc_id;
          my $plat_partnum;
          my $plat_partnum_row=2;
          my $plat_patnum_cul=1;
          $plat_partnum = &lin_xls_cell_value($plat_sheet, $plat_partnum_row, $plat_patnum_cul);
          #print "platnum:$plat_partnum\n";
          while(defined($plat_partnum) or die print "")
          {
             ++$plat_partnum_row;
             $plat_partnum = &lin_xls_cell_value($plat_sheet, $plat_partnum_row, $plat_patnum_cul);
             unless (defined $plat_partnum)
            {
              #print "[$row][scan_idx]No Value, $eos_flag\n" if $DebugPrint == 1 ;
              #$eos_flag -- ;
              $emmc_id="no id";
              next ;
             }
            if ($plat_partnum eq "")
            {
               # print "[$row][scan_idx]EQ null, $eos_flag\n" if $DebugPrint == 1 ;
                # $eos_flag -- ;
               $emmc_id="no id";
               last ;
             }
            $plat_partnum =~ s/^\s+// ;
	    $plat_partnum =~ s/\s+$// ;
             if($plat_partnum eq $part_num)
             {
               $emmc_id = &lin_xls_cell_value($plat_sheet, $plat_partnum_row, 5);
               if ($emmc_id eq "")
               {
                  $emmc_id="no id";
               }
               print "debug:$plat_partnum ,$emmc_id\n";
               last;

             } 
             #print "$plat_partnum_row\n";
               
          }  
          if($emmc_id ne "no id")
         {
           print CONFIGFILE "    <LAYOUT partNumber=\"$part_num\" id=\"$emmc_id\" boot1Size=\"0x$x_boot1\" boot2Size=\"0x$x_boot2\" rpmbSize=\"0x$x_rpmb\" userSize=\"0x$x_user\" gp1Size=\"0x0\" gp2Size=\"0x0\" gp3Size=\"0x0\" gp4Size=\"0x0\"\/>\n";

        }   
    }
    else
    {
         #lenovo-sw jixj 2013.10.23 add begin
         print "not find region name = $part_num\n";
         #die;
         #lenovo-sw jixj 2013.10.23 add end
         my $platsheet="mt$temp_plat";
         my $plat_sheet = get_sheet($platsheet,$EMMC_RegionBook);
         my $emmc_discrete;
          my $plat_partnum;
          #lenovo-sw jixj 2013.10.23 modified begin
          #my $plat_partnum_row=3;
          my $plat_partnum_row=2;
          #lenovo-sw jixj 2013.10.23 modified end
          my $plat_patnum_cul=1;
          $plat_partnum = &lin_xls_cell_value($plat_sheet, $plat_partnum_row, $plat_patnum_cul);
          print "platnum:$part_num\n";
          while(defined($plat_partnum))
          {
             ++$plat_partnum_row;
             $plat_partnum = &lin_xls_cell_value($plat_sheet, $plat_partnum_row, $plat_patnum_cul);
             unless (defined $plat_partnum)
            {
              #print "[$row][scan_idx]No Value, $eos_flag\n" if $DebugPrint == 1 ;
              #$eos_flag -- ;
              next ;
             }
            if ($plat_partnum eq "")
            {
               # print "[$row][scan_idx]EQ null, $eos_flag\n" if $DebugPrint == 1 ;
                # $eos_flag -- ;
               last ;
             }
            $plat_partnum =~ s/^\s+// ;
	    $plat_partnum =~ s/\s+$// ;
             if($plat_partnum eq $part_num)
             {
               $emmc_discrete = &lin_xls_cell_value($plat_sheet, $plat_partnum_row, 2);
               my $t_discrete=substr($emmc_discrete,0,8);
               #print "discrete test is $t_discrete\n";
               if ($t_discrete eq "Discrete")
               {
                  
                  #$emmc_discrete="discrete";
                  print "$part_num memory is discrete,please manual config emmc info in emmc.layout.xml!\n";;
               }
               
               last;

             } 
             #print "$plat_partnum_row\n";
               
          } 
    }

}
print CONFIGFILE "<\/emmc-info>"; 
close CONFIGFILE;
my $plat_path;
if(($task_dir_Pla eq "MT6577") ||($task_dir_Pla eq "MT6589"))
{
  $plat_path=$task_dir_Pla."_Android_scatter_emmc.txt";
}
if(($task_dir_Pla eq "MT6572") ||($task_dir_Pla eq "MT6582")||($task_dir_Pla eq "MT6592"))
{
  $plat_path=$task_dir_Pla."_Android_scatter.txt";
}
  
#my $test=system ("cd $cwd/mediatek/build/tools/Mtx && ./MTXGenExe -g --scatter $cwd/out/target/product/$task_dir_pro/$plat_path --platform $task_dir_Pla");
#print "test is $test";
if(system ("cd $cwd/out/target/product/$task_dir_pro/MTXGen && ./MTXGenExe -g --scatter $cwd/out/target/product/$task_dir_pro/$plat_path --platform $task_dir_Pla")==0)
{
  print "mtxgen generate config file OK!\n";
  
}
else
{
   print "mtxgen generate config file Fail!\n";
   die;
}
if(system ("cd $cwd/out/target/product/$task_dir_pro/MTXGen && ./MTXGenExe -a -f $cwd/out/target/product/$task_dir_pro/MTXGen/config/$task_dir_Pla.cfg.xml")==0)
{
   print "mtxgen generate MTX OK!\n";
}
else
{
  print "mtxgen generate MTX file Fail!\n";
  die;
}

#Get_sheet()
#****************************************************************************************
sub get_sheet {
  my ($sheetName,$Book) = @_;
  print "sheet name is :$sheetName\n"; 
  return $Book->Worksheet($sheetName);
}
#open_excel($ett_calc_table_excel);

#close_excel($ett_calc_table_excel);
#****************************************************************************************
# subroutine:  xls_cell_value
# return:      Excel cell value no matter it's in merge area or not, and in windows or not
# input:       $Sheet:  Specified Excel Sheet
# input:       $row:    Specified row number
# input:       $col:    Specified column number
#****************************************************************************************
sub xls_cell_value {
	my ($Sheet, $row, $col,$SheetName) = @_;
	my $cell = $Sheet->get_cell($row, $col);
	if(defined $cell){
	{
             return  $cell->Value();
        }
  	}else{
		my $error_msg="ERROR in mtx.pl: (row=$row,col=$col) undefine in $SheetName!\n";
		print $error_msg; 
		die $error_msg;
            
	}
}
sub open_excel 
{
    my $ett_calc_table_excel = $_[0];
    
    #print "$ett_calc_table_excel \n";
    chomp($ett_calc_table_excel);

    my $parser = Spreadsheet::ParseExcel->new()or die "can not open excel object:$!";
    $book = $parser->Parse($ett_calc_table_excel)or die "open excel failed:$!"; 
    
    #$Sheet  = $Book->Worksheets($ddr_sheet_name);
    #$cell = $Sheet->get_cell($row, $col);
    return $book
}
sub close_excel
{
}
sub GenConfigFile ()
{
   my $configfile="$cwd/config.xml";
  
   open (CONFIGFILE, ">$configfile") or &error_handler("mtxgen open configfile fail!\n", __FILE__, __LINE__);
   print CONFIGFILE "\nPartnumber=$REGION_TABLE{$part_num}:Boot1=$REGION_TABLE{$part_num}{BOOT1};Boot2=$REGION_TABLE{$part_num}{BOOT2};RPMB=$REGION_TABLE{$part_num}{RPMB};USER=$REGION_TABLE{$part_num}{USER}\n\n" ;
   Close CONFIGFILE;
}
sub lin_xls_cell_value
{
  my ($Sheet, $row, $col) = @_;
  my $cell = $Sheet->get_cell($row, $col);
  return "" unless (defined $cell);
  my $value = $cell->Value();

}


