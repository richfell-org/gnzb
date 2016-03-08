#include <stdio.h>
#include <string.h>

const unsigned long GiB = 1073741824;
const unsigned long MiB = (1024 * 1024);
const unsigned long KiB = 1024;

const unsigned long GB = 1000000000;
const unsigned long MB = (1000 * 1000);
const unsigned long KB = 1000;

unsigned long formatULongBytes(char *out, unsigned long number, bool addSuffix/* = true*/)
{
	unsigned long result;
	if(GiB <= number)
	{
		sprintf(out, "%.02f%s", (double)number /  (double)GiB, (addSuffix ? " GiB" : ""));
		result = GiB;
	}
	else if(MiB <= number)
	{
		sprintf(out, "%.02f%s", (double)number /  (double)MiB, (addSuffix ? " MiB" : ""));
		result = MiB;
	}
	else if(KiB <= number)
	{
		sprintf(out, "%.02f%s", (double)number /  (double)KiB, (addSuffix ? " KiB" : ""));
		result = KiB;
	}
	else
	{
		sprintf(out, "%lu%s", number, (addSuffix ? " Bytes" : ""));
		result = 1;
	}
	return result;
}

unsigned long formatUIntBytes(char *out, unsigned int number, bool addSuffix/* = true*/)
{
	return formatULongBytes(out, (unsigned long)number, addSuffix);
}

unsigned long formatLongBytes(char *out, long number, bool addSuffix/* = true*/)
{
	unsigned long result;
	if((GiB <= number) || ((0 > number) && (GiB <= -number)))
	{
		sprintf(out, "%.2f%s", (double)number / (double)GiB, (addSuffix ? " GiB" : ""));
		result = GiB;
	}
	else if((MiB <= number) || ((0 > number) && (MiB <= -number)))
	{
		sprintf(out, "%.2f%s", (double)number / (double)MiB, (addSuffix ? " MiB" : ""));
		result = MiB;
	}
	else if((KiB <= number) || ((0 > number) && (KiB <= -number)))
	{
		sprintf(out, "%.2f%s", (double)number / (double)KiB, (addSuffix ? " KiB" : ""));
		result = KiB;
	}
	else
	{
		sprintf(out, "%ld%s", number, (addSuffix ? " Bytes" : ""));
		result = 1;
	}
	return result;
}

unsigned long formatIntBytes(char *out, int number, bool addSuffix/* = true*/)
{
	return formatLongBytes(out, (long)number, addSuffix);
}

unsigned long formatBytePerSecond(char *out, unsigned long bytes, unsigned long seconds)
{
	unsigned long result;
	double number = (double)bytes / (double)seconds;
	if(GiB <= number)
	{
		sprintf(out, "%.02f GB/s", number / (double)GB);
		result = GiB;
	}
	else if(MiB <= number)
	{
		sprintf(out, "%.02f MB/s", number /  (double)MB);
		result = MiB;
	}
	else if(KiB <= number)
	{
		sprintf(out, "%.02f KB/s", number /  (double)KB);
		result = KiB;
	}
	else
	{
		sprintf(out, "%.02f B/s", number);
		result = 1;
	}
	return result;
}

unsigned long formatBytePerSecond(char *out, double bytesPerSecond)
{
	unsigned long result;
	if(GiB <= (unsigned long)bytesPerSecond)
	{
		sprintf(out, "%.02f GB/s", bytesPerSecond / (double)GB);
		result = GiB;
	}
	else if(MiB <= (unsigned long)bytesPerSecond)
	{
		sprintf(out, "%.02f MB/s", bytesPerSecond /  (double)MB);
		result = MiB;
	}
	else if(KiB <= (unsigned long)bytesPerSecond)
	{
		sprintf(out, "%.02f KB/s", bytesPerSecond /  (double)KB);
		result = KiB;
	}
	else
	{
		sprintf(out, "%.02f B/s", bytesPerSecond);
		result = 1;
	}
	return result;
}

const unsigned long minutes = 60;
const unsigned long hours = minutes * 60;
const unsigned long days = hours * 24;

void formatTimeFromSeconds(char *out, unsigned long seconds)
{
	unsigned long value;
	int istr = 0;

	*out = 0;
	if(0 == seconds)
		strcpy(out, "<1sec");
	else
	{
		if(seconds >= days)
		{
			value = seconds / days;
			istr += sprintf(&out[istr], "%lddays", value);
			seconds -= (value * days);
		}
		if(seconds >= hours)
		{
			value = seconds / hours;
			istr += sprintf(&out[istr], "%s%02ldhr", (istr > 0 ? ", " : ""), value);
			seconds -= (value * hours);
		}
		if(seconds >= minutes)
		{
			value = seconds / minutes;
			istr += sprintf(&out[istr], "%s%02ldmin", (istr > 0 ? ", " : ""), value);
			seconds -= (value * minutes);
		}
		if(0 < seconds)
			sprintf(&out[istr], "%s%02ldsec", (istr > 0 ? ", " : ""), seconds);
	}
}