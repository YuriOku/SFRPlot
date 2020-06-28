#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#define HUBBLE 0.678

typedef struct {
	double ScaleFactor;
	double Sfr;
} SfrData;

SfrData *data;

char *mopt_command(char *command, int zopt, int Zopt, double boxsize, int lopt);
void oopt_command(double boxsize, int zopt, int Zopt, FILE *gp);

int main(int argc, char *argv[])
{
	int *filelen, *fileoffset;
	int datalen = 0;
	int length = 0;
	int mopt = 0;
	int zopt = 0;
	int Zopt = 0;
	int lopt = 0;
	int oopt = 0;
	int sopt = 0;
	double boxsize = 0;
	double zmax = 0;
	int i, j, k, index, count, bin_num, opt;
	double Time,TotalSM,TotSfr,SfrInMsunPerYear,TotalSumMassStars;
	double amin, amax, amin_i, amax_i, sfrmin, sfrmax;
	double amed, sum_sfr, ave_sfr;
	double bin_width;
	FILE *fp, *gp;
	char buf[256];
	char plot_command[1024] = {'\0'};
	char plot_command_buf[1024];
	char *figname = NULL;
	char *sfrplot_binwidth;

/////////////////////////////////////////////////////////////////////////////////
	while ((opt = getopt(argc, argv, "mb:zZ:los:h")) != -1){
		switch (opt){
			case 'm':
				mopt = 1;
				break;
			case 'b':
				boxsize = atof(optarg);
				break;
			case 'z':
				zopt = 1;
				break;
			case 'Z':
				Zopt = 1;
				zmax = atof(optarg);
				break;
			case 'l':
				lopt = 1;
				break;
			case 'o':
				oopt = 1;
				break;
			case 's':
				sopt = 1;
				figname = optarg;
				break;
			case 'h':
				printf("Usage:\n");
				printf("\t-m: plot with Madau & Dickinson (2014) (require b option)\n");
				printf("\t-o: plot with observational data (require bz option)\n");
				printf("\t-b: boxsize (Unit: Mpc/h)\n");
				printf("\t-l: plot SFR in log scale\n");
				printf("\t-z: set x axis redshift\n");
				printf("\t-Z: set x axis redshift and set max of xrange\n");
				printf("\t-s: save figure (png only)\n");
				printf("\n\teg) sfrplot -mob 10 -lZ 15 -s sfr.png data1/sfr.txt data2/sfr.txt\n");
				exit(0);
			default:
				printf("Error: An unknown option is appointed.\n");
				exit(1);
		}
	}
/////////////////////////////////////////////////////////////////////////////////
  
  if(argc < optind+1)
    {
      printf("Please type sfr file name\n");
      exit(1);
    }
	filelen = (int *)malloc(sizeof(int) * (argc-optind));
	fileoffset = (int *)malloc(sizeof(int) * (argc-optind));
	if (filelen == NULL){
		printf("cannot allocate filelen array\n");
		exit(1);
	}
	if (fileoffset == NULL){
		printf("cannot allocate fileoffset array\n");
		exit(1);
	}
    
	for(i = optind; i < argc; i++){
		//printf("loading %s\n",argv[i]);
		if ((fp = fopen(argv[i],"r")) == NULL){
			printf("file not found\n");
			return -1;
		}
		
		while (fgets(buf, 256, fp) != NULL){
			length++;
		}
		fclose(fp);
		fileoffset[i-optind] = datalen;
		filelen[i-optind] = length;
		datalen += filelen[i-optind];
	}

	printf("data length = %d\n",datalen);
/////////////////////////////////////////////////////////////////////////////////

	//printf("%d MB memory needed to allocate data array\n",(int)(sizeof(SfrData *)*datalen/1000000));
	data = (SfrData *)malloc(sizeof(SfrData) * datalen);
	if (data == NULL){
		printf("cannot allocate data array\n");
		exit(1);
	}
	else{
		//printf("made data array\n");
	}

	for (i = optind; i < argc; i++){
		if ((fp = fopen(argv[i],"r")) == NULL){
			printf("file open failed\n");
			return -1;
		}
		for (j = 0; j < filelen[i-optind]; j++){
			index = fileoffset[i-optind] + j;
			if (index >= datalen){
				printf("something wrong\n");
				return -1;
			}
			fscanf(fp, "%lf %lf %lf %lf %lf",&Time,&TotalSM,&TotSfr,&SfrInMsunPerYear,&TotalSumMassStars);
			//printf("%lf %lf %lf %lf %lf\n",Time,TotalSM,TotSfr,SfrInMsunPerYear,TotalSumMassStars);
			data[index].ScaleFactor = Time;
			data[index].Sfr = SfrInMsunPerYear;
		}	
	}
/////////////////////////////////////////////////////////////////////////////////

	amin = amax = sfrmin = sfrmax = 0.0;
	for (i = 0; i < datalen; i++){
		if (data[i].ScaleFactor > amax) amax = data[i].ScaleFactor;	
		if (data[i].ScaleFactor < amin) amin = data[i].ScaleFactor;	
		if (data[i].Sfr > sfrmax) sfrmax = data[i].Sfr;	
		if (data[i].Sfr < sfrmin) sfrmin = data[i].Sfr;	
	}
	if ((sfrplot_binwidth = getenv("SFRPLOT_BINWIDTH")) == NULL){
		printf("set bin width by environment variable SFRPLOT_BINWIDTH\n");
		exit(1);
	}
/////////////////////////////////////////////////////////////////////////////////

	sprintf(plot_command,"p '-' w l title '%s'",argv[optind]);
	for (i = optind+1; i < argc; i++){
		sprintf(plot_command_buf,",'-' w l title '%s'",argv[i]);
		strcat(plot_command,plot_command_buf);
	}
	if (oopt == 1){
		if (lopt == 1){
			sprintf(plot_command_buf,",'-' u 1:2:3 w e pt 6 title 'observational data'");
			strcat(plot_command,plot_command_buf);
		}
		else{
			sprintf(plot_command_buf,",'-' u 1:(10**$2):(10**($2+$3)-10**$2) w e pt 6 title 'observational data'");
			strcat(plot_command,plot_command_buf);
		}
	}
	if (mopt == 1){
		char command1[256] = {"\0"};
		strcat(plot_command,mopt_command(command1, zopt, Zopt, boxsize, lopt));
	}
	strcat(plot_command,"\n");
/////////////////////////////////////////////////////////////////////////////////
	if (sopt == 0){
		gp = popen("gnuplot -persist","w");
	}
	else{
		gp = popen("gnuplot","w");
		fprintf(gp,"set term png\n");
		fprintf(gp,"set output '%s'\n",figname);
	}
	fprintf(gp,"set xlabel 'time'\n");
	fprintf(gp,"set ylabel 'SFR (Msun/yr)\n");
	//fprintf(gp,"set xrange [%f:%f]\n",amin,amax);
	//fprintf(gp,"set yrange [%f:%f]\n",sfrmin,sfrmax);
	if (zopt == 1){
		fprintf(gp,"set logscale x\n");
		fprintf(gp,"set xlabel 'redshift'\n");
		fprintf(gp,"set xrange [%f:%f]\n",1.0/amax-1.0,10.0);
		fprintf(gp,"set xtics (1,2,3,4,5,6,7,8,9,10,12,14,16,18,20)\n");
	}
	if (Zopt == 1){
		fprintf(gp,"set logscale x\n");
		fprintf(gp,"set xlabel 'redshift'\n");
		fprintf(gp,"set xrange [%f:%f]\n",1.0/amax-1.0,zmax);
		fprintf(gp,"set xtics (1,2,3,4,5,6,7,8,9,10,12,14,16,18,20)\n");
	}
	if (boxsize > 0){
		fprintf(gp,"set ylabel 'SFR density (Msun/yr/Mpc^3)'\n");
	}	
	if (lopt == 1){
		fprintf(gp,"set ylabel 'log SFR (Msun/yr)'\n");
	}
	if (boxsize > 0 && lopt == 1){
		fprintf(gp,"set ylabel 'log SFR density (Msun/yr/Mpc^3)'\n");
	}
/////////////////////////////////////////////////////////////////////////////////

	fprintf(gp,plot_command);
	//printf(plot_command);

	for (i = optind; i < argc; i++){
		amin_i = data[fileoffset[i-optind]].ScaleFactor;
		amax_i = data[fileoffset[i-optind]+filelen[i-optind]-1].ScaleFactor;
		bin_width = atof(sfrplot_binwidth);
		bin_num = (int)((amax_i - amin_i)/bin_width);
		bin_width = (amax_i - amin_i)/(bin_num*1.0);
		//printf("i %d bin_num %d bin_width %f amin %f amax %f \n", i, bin_num, bin_width, amin_i, amax_i);
		for (k = 0; k < bin_num; k++){
			amed = amin_i + (k*1.0+0.5)*bin_width;
			count = 0;
			sum_sfr = ave_sfr = 0.0;
			for (j = fileoffset[i-optind]; j < fileoffset[i-optind]+filelen[i-optind];j++){
				if (amin_i+k*bin_width < data[j].ScaleFactor && data[j].ScaleFactor < amin_i+(k+1)*bin_width){
					count++;
					sum_sfr += data[j].Sfr;
				}
			}
			ave_sfr = sum_sfr/count*1.0;

			if (boxsize > 0){
				ave_sfr = ave_sfr/pow(boxsize/HUBBLE,3.0);
			}
			if (zopt == 1 || Zopt == 1){
				amed = 1.0/amed - 1.0;
			}
			if (lopt ==1){
				ave_sfr = log10(ave_sfr);
			}

			fprintf(gp,"%f\t%f\n",amed,ave_sfr);
		}
		fprintf(gp,"e\n");
	}
	if (oopt == 1){
		oopt_command(boxsize, zopt, Zopt, gp);
	}
	fflush(gp);
	pclose(gp);

	free(filelen);
	free(fileoffset);
  free(data);
	printf("done.\n");
  return 0;
}
/////////////////////////////////////////////////////////////////////////////////

char *mopt_command(char *command, int zopt, int Zopt, double boxsize, int lopt){
	char l0[16];
	char l1[16];
	char x[16];
	if (boxsize == 0){
		printf("enter boxsize with -b option\n");
		exit(1);
	}

	if (zopt == 1 || Zopt == 1){
		sprintf(x,"x");
	}
	else{
		sprintf(x,"(1/x - 1)");
	}

	if (lopt == 1){
		sprintf(l0,"log10(");
		sprintf(l1,")");
	}
	else{
		sprintf(l0," ");
		sprintf(l1," ");
	}

	sprintf(command,", %s0.015*(1+%s)**2.7/(1+((1+%s)/2.9)**5.6)%s \
	w l title 'Madau & Dickinson (2014)",l0,x,x,l1);
	return command;
}

void oopt_command(double boxsize, int zopt, int Zopt, FILE *gp){
	if (boxsize == 0){
		printf("enter boxsize with -b option\n");
		exit(1);
	}
	if (zopt == 0 && Zopt == 0){
		printf("add -z option\n");
		exit(1);
	}

	fprintf(gp,"2.2 -0.74416 0.19 0.19 \n");
	fprintf(gp,"0.2 -1.75 0.13 0.13\n");
	fprintf(gp,"0.47 -1.62 0.13 0.13\n");
	fprintf(gp,"0.75 -1.41 0.13 0.13\n");
	fprintf(gp,"1.1 -1.25 0.17 0.17\n");
	fprintf(gp,"0.3 -1.75 0.13 0.13\n");
	fprintf(gp,"0.5 -1.63 0.13 0.13\n");
	fprintf(gp,"0.7 -1.37 0.13 0.13\n");
	fprintf(gp,"0.9 -1.33 0.17 0.17\n");
	fprintf(gp,"1.1 -1.15 0.17 0.17\n");
	fprintf(gp,"1.3 -1.05 0.17 0.17\n");
	fprintf(gp,"1.5 -0.93 0.17 0.17\n");
	fprintf(gp,"1.7 -1 0.19 0.19\n");
	fprintf(gp,"1.9 -1.05 0.19 0.19\n");
	fprintf(gp,"2.25 -1.23 0.19 0.19\n");
	fprintf(gp,"2.75 -1.23 0.19 0.19\n");
	fprintf(gp,"3.25 -1.45 0.27 0.27\n");
	fprintf(gp,"3.8 -1.95 0.27 0.27\n");
	fprintf(gp,"0.75 -1.22012189825617 0.13 0.13\n");
	fprintf(gp,"1.25 -1.15361632918473 0.17 0.17\n");
	fprintf(gp,"2 -1.02361632918473 0.19 0.19\n");
	fprintf(gp,"3 -1.0787927454597 0.27 0.27\n");
	fprintf(gp,"3.8 -1.31 0.27 0.27\n");
	fprintf(gp,"5.0 -1.71 0.27 0.27\n");
	fprintf(gp,"5.9 -1.92 0.27 0.27\n");
	fprintf(gp,"6.8 -2.14 0.27 0.27\n");
	fprintf(gp,"8.0 -2.37 0.27 0.27\n");
	fprintf(gp,"0.3 -1.72124639904717 0.13 0.13\n");
	fprintf(gp,"0.5 -1.60205999132796 0.13 0.13\n");
	fprintf(gp,"0.7 -1.36653154442041 0.13 0.13\n");
	fprintf(gp,"0.9 -1.23657200643706 0.17 0.17\n");
	fprintf(gp,"1.1 -1.13667713987954 0.17 0.17\n");
	fprintf(gp,"1.4 -1.15490195998574 0.17 0.17\n");
	fprintf(gp,"1.8 -0.924453038607469 0.19 0.19\n");
	fprintf(gp,"2.25 -0.939302159646388 0.19 0.19\n");
	fprintf(gp,"2.75 -0.756961951313705 0.19 0.19\n");
	fprintf(gp,"0.025 -2.03 0.13 0.13\n");
	fprintf(gp,"0.13 -1.8 0.13 0.13\n");
	fprintf(gp,"0.28 -1.63 0.13 0.13\n");
	fprintf(gp,"0.44 -1.46 0.13 0.13\n");
	fprintf(gp,"0.58 -1.27 0.13 0.13\n");
	fprintf(gp,"0.87 -1.12 0.13 0.13\n");
	fprintf(gp,"1.17 -0.81 0.17 0.17\n");
	fprintf(gp,"1.4 -0.753327666658611 0.17 0.17\n");
	fprintf(gp,"0.1 -1.96955107862173 0.13 0.13\n");
	fprintf(gp,"0.5 -1.38708664335714 0.13 0.13\n");
	fprintf(gp,"1 -1.2 0.17 0.17\n");
	fprintf(gp,"2 -0.898970004336019 0.19 0.19\n");
	fprintf(gp,"3 -1.2 0.27 0.27\n");
	fprintf(gp,"4 -1.42184874961636 0.27 0.27\n");
	fprintf(gp,"5 -1.59794000867204 0.27 0.27\n");
	fprintf(gp,"4 -0.9 0.27 0.27\n");
	fprintf(gp,"4.7 -1.3 0.27 0.27\n");
	fprintf(gp,"0.1 -1.84361632918473 0.13 0.13\n");
	fprintf(gp,"2 -1.07361632918473 0.19 0.19\n");
	fprintf(gp,"0.05 -1.89119628734557 0.13 0.13\n");
	fprintf(gp,"0.8 -1.27620641193895 0.13 0.13\n");
	fprintf(gp,"0.5 -1.75 0.13 0.13\n");
	fprintf(gp,"1.0 -1.27 0.17 0.17\n");
	fprintf(gp,"1.5 -1.32 0.17 0.17\n");
	fprintf(gp,"2 -1.23 0.19 0.19\n");
	fprintf(gp,"3 -1.1 0.27 0.27\n");
	fprintf(gp,"4 -1.3 0.27 0.27\n");
	fprintf(gp,"5 -1.8 0.27 0.27\n");
	fprintf(gp,"0.3 -1.7 0.13 0.13\n");
	fprintf(gp,"0.5 -1.55 0.13 0.13\n");
	fprintf(gp,"0.7 -1.2 0.13 0.13\n");
	fprintf(gp,"0.9 -1.15 0.17 0.17\n");
	fprintf(gp,"0.1 -1.88 0.13 0.13\n");
	fprintf(gp,"0.3 -1.67 0.13 0.13\n");
	fprintf(gp,"0.5 -1.57 0.13 0.13\n");
	fprintf(gp,"0.7 -1.38 0.13 0.13\n");
	fprintf(gp,"0.9 -1.13 0.17 0.17\n");
	fprintf(gp,"1.1 -1.08 0.17 0.17\n");
	fprintf(gp,"1.4 -1.08 0.17 0.17\n");
	fprintf(gp,"2.15 -0.85 0.19 0.19\n");
	fprintf(gp,"3 -1.09 0.27 0.27\n");
	fprintf(gp,"4 -1.6 0.27 0.27\n");
	fprintf(gp,"0.4 -1.75332766665861 0.13 0.13\n");
	fprintf(gp,"0.84 -1.23 0.13 0.13\n");
	fprintf(gp,"1.47 -1.117 0.17 0.17\n");
	fprintf(gp,"2.23 -0.91 0.19 0.19\n");
	fprintf(gp,"e\n");

}
