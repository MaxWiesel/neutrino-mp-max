/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2011 CoolStream International Ltd

	License: GPLv2

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation;

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <png.h>
#include <zlib.h>

#include <global.h>
#include <neutrino.h>
#include <gui/filebrowser.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/msgbox.h>
#include <daemonc/remotecontrol.h>
#include <zapit/debug.h>
#include <zapit/getservices.h>
#include <eitd/sectionsd.h>

#include <video.h>
#include <cs_api.h>
#include <driver/screenshot.h>
#include <system/set_threadname.h>
#include <system/helpers.h>

CScreenShot::CScreenShot(const std::string fname, screenshot_format_t fmt)
{
	format = fmt;
	filename = fname;
	xres = 0;
	yres = 0;
	get_osd = ((g_settings.screenshot_mode == 0 ) || (g_settings.screenshot_mode == 2));
	get_video = ((g_settings.screenshot_mode == 1) || (g_settings.screenshot_mode == 2));
	scale_to_osd = ((g_settings.screenshot_mode == 2) && (g_settings.screenshot_scale == 0));
}

CScreenShot::~CScreenShot()
{
}

bool CScreenShot::Start(const std::string custom_cmd)
{
	std::string cmd = find_executable("grab");

	if (cmd.empty())
		return false;

	cmd += " ";

	if (!custom_cmd.empty()) {
		cmd += custom_cmd;
		cmd += " ";
	}

	std::string get = "";
	if (get_osd)
		get = "-o ";
	if (get_video)
		get = "-v ";
	if (get_osd && get_video)
		get = "";
	cmd += get;

	switch (format) {
		default:
		case FORMAT_PNG:
			cmd += "-p ";
			break;
		case FORMAT_JPG:
			cmd += "-j 100 ";
			break;
	}

	if (scale_to_osd)
		cmd += "-d ";

	if (xres)
		cmd += "-w " + to_string(xres) + " ";

	cmd += "'";
	cmd += filename;
	cmd += "'";

	fprintf(stderr, "running: %s\n", cmd.c_str());
	system(cmd.c_str());
	return true;
}

/* 
 * create filename member from channel name and its current EPG data,
 * with added date and time including msecs and suffix for selected format 
 */
void CScreenShot::MakeFileName(const t_channel_id channel_id)
{
	char		fname[512]; // UTF-8
	std::string	channel_name;
	CEPGData	epgData;
	unsigned int	pos = 0;

	snprintf(fname, sizeof(fname), "%s/", g_settings.screenshot_dir.c_str());
	pos = strlen(fname);

	channel_name = CServiceManager::getInstance()->GetServiceName(channel_id);
	if (!(channel_name.empty())) {
		strcpy(&(fname[pos]), UTF8_TO_FILESYSTEM_ENCODING(channel_name.c_str()));
		ZapitTools::replace_char(&fname[pos]);
		strcat(fname, "_");
	}
	pos = strlen(fname);

	if(CEitManager::getInstance()->getActualEPGServiceKey(channel_id, &epgData)) {
		CShortEPGData epgdata;
		if(CEitManager::getInstance()->getEPGidShort(epgData.eventID, &epgdata)) {
			if (!(epgdata.title.empty())) {
				strcpy(&(fname[pos]), epgdata.title.c_str());
				ZapitTools::replace_char(&fname[pos]);
			}
		}
	}
	if (g_settings.screenshot_cover != 1) {
	pos = strlen(fname);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	strftime(&(fname[pos]), sizeof(fname) - pos - 1, "_%Y%m%d_%H%M%S", localtime(&tv.tv_sec));
	pos = strlen(fname);
	snprintf(&(fname[pos]), sizeof(fname) - pos - 1, "_%03d", (int) tv.tv_usec/1000);
	}

	switch (format) {
	default:
		printf("CScreenShot::MakeFileName unsupported format %d, using png\n", format);
		/* fall through */
	case FORMAT_PNG:
		strcat(fname, ".png");
		break;
	case FORMAT_JPG:
		strcat(fname, ".jpg");
		break;
	}
	printf("CScreenShot::MakeFileName: [%s]\n", fname);
	filename = std::string(fname);
}
