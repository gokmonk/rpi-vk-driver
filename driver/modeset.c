#include "modeset.h"

#include <stdatomic.h>
atomic_int saved_state_guard = 0;

void modeset_enum_displays(int fd, uint32_t* numDisplays, modeset_display* displays)
{
	 drmModeResPtr resPtr = drmModeGetResources(fd);

	 uint32_t tmpNumDisplays = 0;
	 modeset_display tmpDisplays[16];

	 for(uint32_t c = 0; c < resPtr->count_connectors; ++c)
	 {
		 drmModeConnectorPtr connPtr = drmModeGetConnector(fd, resPtr->connectors[c]);

		 if(connPtr->connection != DRM_MODE_CONNECTED)
		 {
			 continue; //skip unused connector
		 }

		 if(!connPtr->count_modes)
		 {
			 continue; //skip connectors with no valid modes
		 }

		 memcpy(tmpDisplays[tmpNumDisplays].name, connPtr->modes[0].name, 32);

		 tmpDisplays[tmpNumDisplays].mmWidth = connPtr->mmWidth;
		 tmpDisplays[tmpNumDisplays].mmHeight = connPtr->mmHeight;
		 tmpDisplays[tmpNumDisplays].resWidth = connPtr->modes[0].hdisplay;
		 tmpDisplays[tmpNumDisplays].resHeight = connPtr->modes[0].vdisplay;
		 tmpDisplays[tmpNumDisplays].connectorID = connPtr->connector_id;

		 tmpNumDisplays++;

		 assert(tmpNumDisplays < 16);

		 drmModeFreeConnector(connPtr);
	 }

	 drmModeFreeResources(resPtr);

	 *numDisplays = tmpNumDisplays;

	 memcpy(displays, tmpDisplays, tmpNumDisplays * sizeof(modeset_display));
}

void modeset_enum_modes_for_display(int fd, uint32_t display, uint32_t* numModes, modeset_display_mode* modes)
{
	drmModeResPtr resPtr = drmModeGetResources(fd);

	drmModeConnectorPtr connPtr = drmModeGetConnector(fd, display);

	if(!connPtr)
	{
		*numModes = 0;
		return;
	}

	uint32_t tmpNumModes = 0;
	modeset_display_mode tmpModes[1024];

	for(uint32_t c = 0; c < connPtr->count_modes; ++c)
	{
		uint32_t found = 0;
		for(uint32_t d = 0; d < tmpNumModes; ++d)
		{
			if(tmpModes[d].refreshRate == connPtr->modes[c].vrefresh &&
			   tmpModes[d].resWidth == connPtr->modes[c].hdisplay &&
			   tmpModes[d].resHeight == connPtr->modes[c].vdisplay)
			{
				found = 1;
				break;
			}
		}

		if(found)
		{
			//skip modes that have lower "clock"
			//but otherwise same resolution and vrefresh
			continue;
		}

		tmpModes[tmpNumModes].connectorID = display;
		tmpModes[tmpNumModes].modeID = c;
		tmpModes[tmpNumModes].refreshRate = connPtr->modes[c].vrefresh;
		tmpModes[tmpNumModes].resWidth = connPtr->modes[c].hdisplay;
		tmpModes[tmpNumModes].resHeight = connPtr->modes[c].vdisplay;

		//explained here:
		// https://01.org/linuxgraphics/gfx-docs/drm/API-struct-drm-display-mode.html

//		char* typestr = 0;
//		switch(connPtr->modes[c].type)
//		{
//		case DRM_MODE_TYPE_BUILTIN:
//			typestr = "DRM_MODE_TYPE_BUILTIN";
//			break;
//		case DRM_MODE_TYPE_CLOCK_C:
//			typestr = "DRM_MODE_TYPE_CLOCK_C";
//			break;
//		case DRM_MODE_TYPE_CRTC_C:
//			typestr = "DRM_MODE_TYPE_CRTC_C";
//			break;
//		case DRM_MODE_TYPE_PREFERRED:
//			typestr = "DRM_MODE_TYPE_PREFERRED";
//			break;
//		case DRM_MODE_TYPE_DEFAULT:
//			typestr = "DRM_MODE_TYPE_DEFAULT";
//			break;
//		case DRM_MODE_TYPE_USERDEF:
//			typestr = "DRM_MODE_TYPE_USERDEF";
//			break;
//		case DRM_MODE_TYPE_DRIVER:
//			typestr = "DRM_MODE_TYPE_DRIVER";
//			break;
//		default:
//			typestr = "UNKNOWN";
//			break;
//		}

//		fprintf(stderr, "\nflags ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_PHSYNC) fprintf(stderr, "DRM_MODE_FLAG_PHSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_NHSYNC) fprintf(stderr, "DRM_MODE_FLAG_NHSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_PVSYNC) fprintf(stderr, "DRM_MODE_FLAG_PVSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_NVSYNC) fprintf(stderr, "DRM_MODE_FLAG_NVSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_INTERLACE) fprintf(stderr, "DRM_MODE_FLAG_INTERLACE ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_DBLSCAN) fprintf(stderr, "DRM_MODE_FLAG_DBLSCAN ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_CSYNC) fprintf(stderr, "DRM_MODE_FLAG_CSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_PCSYNC) fprintf(stderr, "DRM_MODE_FLAG_PCSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_NCSYNC) fprintf(stderr, "DRM_MODE_FLAG_NCSYNC ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_HSKEW) fprintf(stderr, "DRM_MODE_FLAG_HSKEW ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_BCAST) fprintf(stderr, "DRM_MODE_FLAG_BCAST ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_PIXMUX) fprintf(stderr, "DRM_MODE_FLAG_PIXMUX ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_DBLCLK) fprintf(stderr, "DRM_MODE_FLAG_DBLCLK ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_CLKDIV2) fprintf(stderr, "DRM_MODE_FLAG_CLKDIV2 ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_MASK) fprintf(stderr, "DRM_MODE_FLAG_3D_MASK ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_NONE) fprintf(stderr, "DRM_MODE_FLAG_3D_NONE ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_FRAME_PACKING) fprintf(stderr, "DRM_MODE_FLAG_3D_FRAME_PACKING ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE) fprintf(stderr, "DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_LINE_ALTERNATIVE) fprintf(stderr, "DRM_MODE_FLAG_3D_LINE_ALTERNATIVE ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL) fprintf(stderr, "DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_L_DEPTH) fprintf(stderr, "DRM_MODE_FLAG_3D_L_DEPTH ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH) fprintf(stderr, "DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_TOP_AND_BOTTOM) fprintf(stderr, "DRM_MODE_FLAG_3D_TOP_AND_BOTTOM ");
//		if(connPtr->modes[c].flags & DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF) fprintf(stderr, "DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF ");

//		fprintf(stderr, "\nclock %u vrefresh %u type %s\n", connPtr->modes[c].clock, connPtr->modes[c].vrefresh, typestr);
//		fprintf(stderr, "hdisplay %u hsync_start %u hsync_end %u htotal %u hskew %u\n", connPtr->modes[c].hdisplay, connPtr->modes[c].hsync_start, connPtr->modes[c].hsync_end, connPtr->modes[c].htotal, connPtr->modes[c].hskew);
//		fprintf(stderr, "vdisplay %u vsync_start %u vsync_end %u vtotal %u vscan %u\n", connPtr->modes[c].vdisplay, connPtr->modes[c].vsync_start, connPtr->modes[c].vsync_end, connPtr->modes[c].vtotal, connPtr->modes[c].vscan);

		tmpNumModes++;

		assert(tmpNumModes < 1024);
	}

	drmModeFreeConnector(connPtr);
	drmModeFreeResources(resPtr);

	*numModes = tmpNumModes;
	memcpy(modes, tmpModes, tmpNumModes * sizeof(modeset_display_mode));
}

void modeset_create_surface_for_mode(int fd, uint32_t display, uint32_t mode, modeset_display_surface* surface)
{
//	modeset_debug_print(fd);

	surface->savedState = 0;

	drmModeResPtr resPtr = drmModeGetResources(fd);

	drmModeConnectorPtr connPtr = drmModeGetConnector(fd, display);

	if(!connPtr)
	{
		return;
	}

	uint32_t found = 0;

	//if current encoder is valid, try to use that
	if(connPtr->encoder_id)
	{
		drmModeEncoderPtr encPtr = drmModeGetEncoder(fd, connPtr->encoder_id);

		if(encPtr && encPtr->crtc_id)
		{
			surface->connector = connPtr;
			surface->modeID = mode;
			surface->crtc = drmModeGetCrtc(fd, encPtr->crtc_id);

			found = 1;
		}

		drmModeFreeEncoder(encPtr);
	}

	if(!found)
	{
//		To find a suitable CRTC, you need to iterate over the list of encoders that are available
//		for each connector. Each encoder contains a list of CRTCs that it can work with and you
//		simply select one of these CRTCs. If you later program the CRTC to control a connector,
//		it automatically selects the best encoder

//		TODO if we were to output to multiple displays, we'd need to make sure we don't use a CRTC
//		that we'd be using to drive the other screen

		for(uint32_t c = 0; c < connPtr->count_encoders; ++c)
		{
			drmModeEncoderPtr encPtr = drmModeGetEncoder(fd, connPtr->encoders[c]);

			for(uint32_t d = 0; d < 32; ++d)
			{
				if(encPtr->possible_crtcs & (1 << d))
				{
					surface->connector = connPtr;
					surface->modeID = mode;
					surface->crtc = drmModeGetCrtc(fd, resPtr->crtcs[d]);

					found = 1;
					break;
				}
			}

			if(found)
			{
				break;
			}
		}
	}

	drmModeFreeResources(resPtr);

	//fprintf(stderr, "connector id %i, crtc id %i\n", connPtr->connector_id, encPtr->crtc_id);
}

void modeset_create_fb_for_surface(int fd, _image* buf, modeset_display_surface* surface)
{
	int ret = drmModeAddFB(fd, buf->width, buf->height, 24, 32, buf->stride, buf->boundMem->bo, &buf->fb);

	if(ret)
	{
		buf->fb = 0;
		fprintf(stderr, "cannot create framebuffer (%d): %m\n", errno);
	}
}

void modeset_destroy_fb(int fd, _image* buf)
{
	// delete framebuffer
	drmModeRmFB(fd, buf->fb);
}

void modeset_present(int fd, _image *buf, modeset_display_surface* surface)
{
	if(!surface->savedState)
	{
		while(saved_state_guard);
		saved_state_guard = 1;

		for(uint32_t c = 0; c < 32; ++c)
		{
			if(!modeset_saved_states[c].used)
			{
				drmModeConnectorPtr tmpConnPtr = drmModeGetConnector(fd, surface->connector->connector_id);
				drmModeCrtcPtr tmpCrtcPtr = drmModeGetCrtc(fd, surface->crtc->crtc_id);
				modeset_saved_states[c].used = 1;
				modeset_saved_states[c].conn = tmpConnPtr;
				modeset_saved_states[c].crtc = tmpCrtcPtr;
				surface->savedState = c;
				break;
			}
		}

		int ret = drmModeSetCrtc(fd, surface->crtc->crtc_id, buf->fb, 0, 0, &surface->connector->connector_id, 1, &surface->connector->modes[surface->modeID]);
		if(ret)
		{
			fprintf(stderr, "cannot flip CRTC for connector %u (%d): %m\n",
				   surface->connector->connector_id, errno);
		}

		saved_state_guard = 0;
	}

	//TODO add vsync support eventually
	drmModePageFlip(fd, surface->crtc->crtc_id, buf->fb, 0, 0);

	//modeset_debug_print(fd);
}

void modeset_destroy_surface(int fd, modeset_display_surface *surface)
{
	//restore old state
	drmModeSetCrtc(fd, modeset_saved_states[surface->savedState].crtc->crtc_id,
			modeset_saved_states[surface->savedState].crtc->buffer_id,
			modeset_saved_states[surface->savedState].crtc->x,
			modeset_saved_states[surface->savedState].crtc->y,
			&modeset_saved_states[surface->savedState].conn->connector_id,
			1,
			&modeset_saved_states[surface->savedState].crtc->mode);

	{
		while(saved_state_guard);
		saved_state_guard = 1;

		drmModeFreeConnector(modeset_saved_states[surface->savedState].conn);
		drmModeFreeCrtc(modeset_saved_states[surface->savedState].crtc);
		modeset_saved_states[surface->savedState].used = 0;

		saved_state_guard = 0;
	}

	drmModeFreeConnector(surface->connector);
	drmModeFreeCrtc(surface->crtc);
}

void modeset_debug_print(int fd)
{
	drmModeResPtr resPtr = drmModeGetResources(fd);
	printf("res min width %i height %i\n", resPtr->min_width, resPtr->min_height);
	printf("res max width %i height %i\n", resPtr->max_width, resPtr->max_height);

	printf("\ncrtc count %i\n", resPtr->count_crtcs);
	for(uint32_t c = 0; c < resPtr->count_crtcs; ++c)
	{
		drmModeCrtcPtr tmpCrtcPtr = drmModeGetCrtc(fd, resPtr->crtcs[c]);
		printf("crtc id %i, buffer id %i\n", tmpCrtcPtr->crtc_id, tmpCrtcPtr->buffer_id);
		drmModeFreeCrtc(tmpCrtcPtr);
	}

	printf("\nfb count %i\n", resPtr->count_fbs);
	for(uint32_t c = 0; c < resPtr->count_fbs; ++c)
	{
	   drmModeFBPtr tmpFBptr = drmModeGetFB(fd, resPtr->fbs[c]);
	   printf("fb id %i, handle %i\n", tmpFBptr->fb_id, tmpFBptr->handle);
	   drmModeFreeFB(tmpFBptr);
	}

	printf("\nencoder count %i\n", resPtr->count_encoders);
	for(uint32_t c = 0; c < resPtr->count_encoders; ++c)
	{
	   drmModeEncoderPtr tmpEncoderPtr = drmModeGetEncoder(fd, resPtr->encoders[c]);
	   printf("encoder id %i, crtc id %i\n", tmpEncoderPtr->encoder_id, tmpEncoderPtr->crtc_id);
	   printf("possible crtcs: ");
	   for(uint32_t c = 0; c < 32; ++c)
	   {
		   if(tmpEncoderPtr->possible_crtcs & (1 << c))
		   {
			   printf("%i, ", c);
		   }
	   }
	   printf("\n");

	   printf("possible clones: ");
	   for(uint32_t c = 0; c < 32; ++c)
	   {
		   if(tmpEncoderPtr->possible_clones & (1 << c))
		   {
			   printf("%i, ", c);
		   }
	   }
	   printf("\n");

	   drmModeFreeEncoder(tmpEncoderPtr);
	}

	printf("\nconnector count %i\n", resPtr->count_connectors);
	for(uint32_t c = 0; c < resPtr->count_connectors; ++c)
	{
		drmModeConnectorPtr tmpConnPtr = drmModeGetConnector(fd, resPtr->connectors[c]);
		printf("connector id %i, encoder id %i\n", tmpConnPtr->connector_id, tmpConnPtr->encoder_id);
		drmModeFreeConnector(tmpConnPtr);
	}

	drmModeFreeResources(resPtr);
}
