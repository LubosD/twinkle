/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// Indices of categories in the category list box
#define idxCatUser	0
#define idxCatSipServer	1
#define idxCatVoiceMail	2
#define idxCatIM		3
#define idxCatPresence	4
#define idxCatRtpAudio	5
#define idxCatSipProtocol	6
#define idxCatNat	7
#define idxCatAddrFmt	8
#define idxCatTimers	9
#define idxCatRingTones	10
#define idxCatScripts	11
#define idxCatSecurity	12

// Indices of call hold variants in the call hold variant list box
#define idxHoldRfc2543	0
#define idxHoldRfc3264	1

// Indices of SIP extension support types in the list box
#define idxExtDisabled	0
#define idxExtSupported	1
#define idxExtRequired	2
#define idxExtPreferred	3

// Indices of RTP audio tabs
#define idxRtpCodecs	0
#define idxRtpPreprocessing 1
#define idxRtpIlbc	    2
#define idxRtpSpeex	    3
#define idxRtpDtmf	    4

// Codec labels
#define labelCodecG711a		"G.711 A-law"
#define labelCodecG711u		"G.711 u-law"
#define labelCodecGSM		"GSM"
#define labelCodecSpeexNb		"speex-nb (8 kHz)"
#define labelCodecSpeexWb	"speex-wb (16 kHz)"
#define labelCodecSpeexUwb	"speex-uwb (32 kHz)"
#define labelCodecIlbc		"iLBC"
#define labelCodecG726_16		"G.726 16 kbps"
#define labelCodecG726_24		"G.726 24 kbps"
#define labelCodecG726_32		"G.726 32 kbps"
#define labelCodecG726_40		"G.726 40 kbps"

// Indices of iLBC modes
#define idxIlbcMode20	0
#define idxIlbcMode30	1

// Indices of G.726 packing modes
#define idxG726PackRfc3551	0
#define idxG726PackAal2		1

// Indices of DTMF transport modes in the DTMF transport list box
#define idxDtmfAuto	0
#define idxDtmfRfc2833	1
#define idxDtmfInband	2
#define idxDtmfInfo		3

// Columns in the number conversion list view
#define colExpr		0
#define colReplace		1

// MWI type indices
#define idxMWIUnsollicited	0
#define idxMWISollicited	1

// SIP transport protocol indices
#define idxSipTransportAuto	0
#define idxSipTransportUDP	1
#define idxSipTransportTCP	2

void UserProfileForm::init()
{
	QRegExp rxNoSpace("\\S*");
	QRegExp rxNoAtSign("[^@]*");
	QRegExp rxQvalue("(0\\.[0-9]{0,3})|(1\\.0{0,3})");
	QRegExp rxAkaOpValue("[a-zA-Z0-9]{0,32}");
	QRegExp rxAkaAmfValue("[a-zA-Z0-9]{0,4}");
	
	// Set validators
	// USER
	domainLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	authAkaOpLineEdit->setValidator(new QRegExpValidator(rxAkaOpValue, this));
	authAkaAmfLineEdit->setValidator(new QRegExpValidator(rxAkaAmfValue, this));
	
	// SIP SERVER
	registrarLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	regQvalueLineEdit->setValidator(new QRegExpValidator(rxQvalue, this));
	proxyLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	
	// Voice mail
	mwiServerLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	
	// NAT
	publicIPLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	
	// Address format
	testConversionLineEdit->setValidator(new QRegExpValidator(rxNoAtSign, this));
	
#ifndef HAVE_SPEEX
	// Speex & (Speex) Preprocessing
	speexGroupBox->hide();
	preprocessingGroupBox->hide();
	rtpAudioTabWidget->setTabEnabled(rtpAudioTabWidget->page(idxRtpSpeex), false);
	rtpAudioTabWidget->setTabEnabled(rtpAudioTabWidget->page(idxRtpPreprocessing), false);
#endif
#ifndef HAVE_ILBC
	// iLBC
	ilbcGroupBox->hide();
	rtpAudioTabWidget->setTabEnabled(rtpAudioTabWidget->page(idxRtpIlbc), false);
#endif
#ifndef HAVE_ZRTP
	// Zrtp
	zrtpEnabledCheckBox->setEnabled(false);
	zrtpSettingsGroupBox->hide();
#endif
	
	// Set toolbutton icons for disabled options.
	QIconSet i;
	i = openRingtoneToolButton->iconSet();
	i.setPixmap(QPixmap::fromMimeSource("fileopen-disabled.png"), 
		    QIconSet::Automatic, QIconSet::Disabled);
	openRingtoneToolButton->setIconSet(i);
	openRingbackToolButton->setIconSet(i);
	openIncomingCallScriptToolButton->setIconSet(i);
}

void UserProfileForm::showCategory( int index )
{
	if (index == idxCatUser) {
		settingsWidgetStack->raiseWidget(pageUser);
	} else if (index == idxCatSipServer) {
		settingsWidgetStack->raiseWidget(pageSipServer);
	} else if (index == idxCatVoiceMail) {
		settingsWidgetStack->raiseWidget(pageVoiceMail);
	} else if (index == idxCatIM) {
		settingsWidgetStack->raiseWidget(pageIM);
	} else if (index == idxCatPresence) {
		settingsWidgetStack->raiseWidget(pagePresence);
	} else if (index == idxCatRtpAudio) {
		settingsWidgetStack->raiseWidget(pageRtpAudio);
	} else if (index == idxCatSipProtocol) {
		settingsWidgetStack->raiseWidget(pageSipProtocol);
	} else if (index == idxCatNat) {
		settingsWidgetStack->raiseWidget(pageNat);
	} else if (index == idxCatAddrFmt) {
		settingsWidgetStack->raiseWidget(pageAddressFormat);
	} else if (index == idxCatTimers) {
		settingsWidgetStack->raiseWidget(pageTimers);
	} else if (index == idxCatRingTones) {
		settingsWidgetStack->raiseWidget(pageRingTones);
	} else if (index == idxCatScripts) {
		settingsWidgetStack->raiseWidget(pageScripts);
	} else if (index == idxCatSecurity) {
		settingsWidgetStack->raiseWidget(pageSecurity);
	}
}

// Convert a label to a codec
t_audio_codec UserProfileForm::label2codec(const QString &label) {
	if (label == labelCodecG711a) {
		return CODEC_G711_ALAW;
	} else if (label == labelCodecG711u) {
		return CODEC_G711_ULAW;
	} else if (label == labelCodecGSM) {
		return CODEC_GSM;
	} else if (label == labelCodecSpeexNb) {
		return CODEC_SPEEX_NB;
	} else if (label == labelCodecSpeexWb) {
		return CODEC_SPEEX_WB;
	} else if (label == labelCodecSpeexUwb) {
		return CODEC_SPEEX_UWB;
	} else if (label == labelCodecIlbc) {
		return CODEC_ILBC;
	} else if (label == labelCodecG726_16) {
		return CODEC_G726_16;
	} else if (label == labelCodecG726_24) {
		return CODEC_G726_24;
	} else if (label == labelCodecG726_32) {
		return CODEC_G726_32;
	} else if (label == labelCodecG726_40) {
		return CODEC_G726_40;
	}
	return CODEC_NULL;
}

// Convert a codec to a label
QString UserProfileForm::codec2label(t_audio_codec &codec) {
	switch (codec) {
	case CODEC_G711_ALAW:
		return labelCodecG711a;
	case CODEC_G711_ULAW:
		return labelCodecG711u;
	case CODEC_GSM:
		return labelCodecGSM;
	case CODEC_SPEEX_NB:
		return labelCodecSpeexNb;
	case CODEC_SPEEX_WB:
		return labelCodecSpeexWb;
	case CODEC_SPEEX_UWB:
		return labelCodecSpeexUwb;
	case CODEC_ILBC:
		return labelCodecIlbc;
	case CODEC_G726_16:
		return labelCodecG726_16;
	case CODEC_G726_24:
		return labelCodecG726_24;
	case CODEC_G726_32:
		return labelCodecG726_32;
	case CODEC_G726_40:
		return labelCodecG726_40;
	default:
		return "";
	}
}

// Convert t_ext_support to an index in the SIP extension combo box
int UserProfileForm::ext_support2indexComboItem(t_ext_support ext) {
	switch(ext) {
	case EXT_DISABLED:
		return idxExtDisabled;
	case EXT_SUPPORTED:
		return idxExtSupported;
	case EXT_REQUIRED:
		return idxExtRequired;
	case EXT_PREFERRED:
		return idxExtPreferred;
	default:
		return idxExtDisabled;
	}
	
	return idxExtDisabled;
}

t_ext_support UserProfileForm::indexComboItem2ext_support(int index) {
	switch(index) {
	case idxExtDisabled:
		return EXT_DISABLED;
	case idxExtSupported:
		return EXT_SUPPORTED;
	case idxExtRequired:
		return EXT_REQUIRED;
	case idxExtPreferred:
		return EXT_PREFERRED;
	}
	
	return EXT_DISABLED;
}

// Populate the form
void UserProfileForm::populate()
{
	QString s;
	
	// Set user profile name in the titlebar
	s = PRODUCT_NAME;
	s.append(" - ").append(tr("User profile:")).append(" ");
	s.append(current_profile->get_profile_name().c_str());
	setCaption(s);
	
	// Select the User category
	categoryListBox->setSelected(idxCatUser, true);
	settingsWidgetStack->raiseWidget(pageUser);
	
	// Set focus on first field
	displayLineEdit->setFocus();
	
	// Set the values of the current_profile object in the form
	// USER
	displayLineEdit->setText(current_profile->get_display(false).c_str());
	usernameLineEdit->setText(current_profile->get_name().c_str());
	domainLineEdit->setText(current_profile->get_domain().c_str());
	organizationLineEdit->setText(current_profile->get_organization().c_str());
	authRealmLineEdit->setText(current_profile->get_auth_realm().c_str());
	authNameLineEdit->setText(current_profile->get_auth_name().c_str());
	authPasswordLineEdit->setText(current_profile->get_auth_pass().c_str());
	
	uint8 aka_op[AKA_OPLEN];
	current_profile->get_auth_aka_op(aka_op);
	authAkaOpLineEdit->setText(binary2hex(aka_op, AKA_OPLEN).c_str());
	
	uint8 aka_amf[AKA_AMFLEN];
	current_profile->get_auth_aka_amf(aka_amf);
	authAkaAmfLineEdit->setText(binary2hex(aka_amf, AKA_AMFLEN).c_str());
	
	// SIP SERVER
	registrarLineEdit->setText(current_profile->get_registrar().encode_noscheme().c_str());
	expirySpinBox->setValue(current_profile->get_registration_time());
	regAtStartupCheckBox->setChecked(current_profile->get_register_at_startup());
	regAddQvalueCheckBox->setChecked(current_profile->get_reg_add_qvalue());
	regQvalueLineEdit->setEnabled(current_profile->get_reg_add_qvalue());
	regQvalueLineEdit->setText(float2str(current_profile->get_reg_qvalue(), 3).c_str());
	useProxyCheckBox->setChecked(current_profile->get_use_outbound_proxy());
	proxyTextLabel->setEnabled(current_profile->get_use_outbound_proxy());
	proxyLineEdit->setEnabled(current_profile->get_use_outbound_proxy());
	if (current_profile->get_use_outbound_proxy()) {
		proxyLineEdit->setText(current_profile->
				       get_outbound_proxy().encode_noscheme().c_str());
	} else {
		proxyLineEdit->clear();
	}
	allRequestsCheckBox->setChecked(current_profile->get_all_requests_to_proxy());
	allRequestsCheckBox->setEnabled(current_profile->get_use_outbound_proxy());
	proxyNonResolvableCheckBox->setChecked(current_profile->get_non_resolvable_to_proxy());
	proxyNonResolvableCheckBox->setEnabled(current_profile->get_use_outbound_proxy());
	
	// VOICE MAIL
	vmAddressLineEdit->setText(current_profile->get_mwi_vm_address().c_str());
	if (current_profile->get_mwi_sollicited()) {
		mwiTypeComboBox->setCurrentItem(idxMWISollicited);
		mwiSollicitedGroupBox->setEnabled(true);
	} else {
		mwiTypeComboBox->setCurrentItem(idxMWIUnsollicited);
		mwiSollicitedGroupBox->setEnabled(false);
	}
	mwiUserLineEdit->setText(current_profile->get_mwi_user().c_str());
	mwiServerLineEdit->setText(current_profile->
				   get_mwi_server().encode_noscheme().c_str());
	mwiViaProxyCheckBox->setChecked(current_profile->get_mwi_via_proxy());
	mwiDurationSpinBox->setValue(current_profile->get_mwi_subscription_time());
	
	// INSTANT MESSAGE
	imMaxSessionsSpinBox->setValue(current_profile->get_im_max_sessions());
	isComposingCheckBox->setChecked(current_profile->get_im_send_iscomposing());
	
	// PRESENCE
	presPublishCheckBox->setChecked(current_profile->get_pres_publish_startup());
	presPublishTimeSpinBox->setValue(current_profile->get_pres_publication_time());
	presSubscribeTimeSpinBox->setValue(current_profile->get_pres_subscription_time());
	
	// RTP AUDIO
	// Codecs
	QStringList allCodecs;
	allCodecs.append(labelCodecG711a);
	allCodecs.append(labelCodecG711u);
	allCodecs.append(labelCodecGSM);
#ifdef HAVE_SPEEX
	allCodecs.append(labelCodecSpeexNb);
	allCodecs.append(labelCodecSpeexWb);
	allCodecs.append(labelCodecSpeexUwb);
#endif
#ifdef HAVE_ILBC
	allCodecs.append(labelCodecIlbc);
#endif
	allCodecs.append(labelCodecG726_16);
	allCodecs.append(labelCodecG726_24);
	allCodecs.append(labelCodecG726_32);
	allCodecs.append(labelCodecG726_40);
	activeCodecListBox->clear();
	list<t_audio_codec> audio_codecs = current_profile->get_codecs();
	for (list<t_audio_codec>::iterator i = audio_codecs.begin(); i != audio_codecs.end(); i++)
	{
		activeCodecListBox->insertItem(codec2label(*i));
		allCodecs.remove(codec2label(*i));
	}
	availCodecListBox->clear();
	if (!allCodecs.empty()) availCodecListBox->insertStringList(allCodecs);
	
	// G.711/G.726 ptime
	ptimeSpinBox->setValue(current_profile->get_ptime());
	
	// Codec preference
	inFarEndCodecPrefCheckBox->setChecked(current_profile->get_in_obey_far_end_codec_pref());
	outFarEndCodecPrefCheckBox->setChecked(current_profile->get_out_obey_far_end_codec_pref());
	
	// Speex preprocessing and AEC
	spxDspVadCheckBox->setChecked(current_profile->get_speex_dsp_vad());
	spxDspAgcCheckBox->setChecked(current_profile->get_speex_dsp_agc());
	spxDspAecCheckBox->setChecked(current_profile->get_speex_dsp_aec());
	spxDspNrdCheckBox->setChecked(current_profile->get_speex_dsp_nrd());
	spxDspAgcLevelSpinBox->setValue(current_profile->get_speex_dsp_agc_level());
	spxDspAgcLevelTextLabel->setEnabled(current_profile->get_speex_dsp_agc());
	spxDspAgcLevelSpinBox->setEnabled(current_profile->get_speex_dsp_agc());
	
	// Speex ([en/de]coding)
	spxVbrCheckBox->setChecked(current_profile->get_speex_bit_rate_type() == BIT_RATE_VBR);
	spxDtxCheckBox->setChecked(current_profile->get_speex_dtx());
	spxPenhCheckBox->setChecked(current_profile->get_speex_penh());
	spxQualitySpinBox->setValue(current_profile->get_speex_quality());
	spxComplexitySpinBox->setValue(current_profile->get_speex_complexity());
	spxNbPayloadSpinBox->setValue(current_profile->get_speex_nb_payload_type());
	spxWbPayloadSpinBox->setValue(current_profile->get_speex_wb_payload_type());
	spxUwbPayloadSpinBox->setValue(current_profile->get_speex_uwb_payload_type());
	
	// iLBC
	ilbcPayloadSpinBox->setValue(current_profile->get_ilbc_payload_type());
	
	if (current_profile->get_ilbc_mode() == 20) {
		ilbcPayloadSizeComboBox->setCurrentItem(idxIlbcMode20);
	} else {
		ilbcPayloadSizeComboBox->setCurrentItem(idxIlbcMode30);
	}
	
	// G.726
	g72616PayloadSpinBox->setValue(current_profile->get_g726_16_payload_type());
	g72624PayloadSpinBox->setValue(current_profile->get_g726_24_payload_type());
	g72632PayloadSpinBox->setValue(current_profile->get_g726_32_payload_type());
	g72640PayloadSpinBox->setValue(current_profile->get_g726_40_payload_type());
	
	if (current_profile->get_g726_packing() == G726_PACK_RFC3551) {
		g726PackComboBox->setCurrentItem(idxG726PackRfc3551);
	} else {
		g726PackComboBox->setCurrentItem(idxG726PackAal2);
	}
	
	// DTMF
	switch (current_profile->get_dtmf_transport()) {
	case DTMF_RFC2833:
		dtmfTransportComboBox->setCurrentItem(idxDtmfRfc2833);
		break;
	case DTMF_INBAND:
		dtmfTransportComboBox->setCurrentItem(idxDtmfInband);
		break;
	case DTMF_INFO:
		dtmfTransportComboBox->setCurrentItem(idxDtmfInfo);
		break;
	default:
		dtmfTransportComboBox->setCurrentItem(idxDtmfAuto);
		break;
	}
	
	dtmfPayloadTypeSpinBox->setValue(current_profile->get_dtmf_payload_type());
	dtmfDurationSpinBox->setValue(current_profile->get_dtmf_duration());
	dtmfPauseSpinBox->setValue(current_profile->get_dtmf_pause());
	dtmfVolumeSpinBox->setValue(-(current_profile->get_dtmf_volume()));
	
	// SIP PROTOCOL
	switch (current_profile->get_hold_variant()) {
	case HOLD_RFC2543:
		holdVariantComboBox->setCurrentItem(idxHoldRfc2543);
		break;
	default:
		holdVariantComboBox->setCurrentItem(idxHoldRfc3264);
		break;
	}
	
	maxForwardsCheckBox->setChecked(current_profile->get_check_max_forwards());
	missingContactCheckBox->setChecked(current_profile->get_allow_missing_contact_reg());
	regTimeCheckBox->setChecked(current_profile->get_registration_time_in_contact());
	compactHeadersCheckBox->setChecked(current_profile->get_compact_headers());
	multiValuesListCheckBox->setChecked(
			current_profile->get_encode_multi_values_as_list());
	useDomainInContactCheckBox->setChecked(
			current_profile->get_use_domain_in_contact());
	allowSdpChangeCheckBox->setChecked(current_profile->get_allow_sdp_change());
	allowRedirectionCheckBox->setChecked(current_profile->get_allow_redirection());
	askUserRedirectCheckBox->setEnabled(current_profile->get_allow_redirection());
	askUserRedirectCheckBox->setChecked(current_profile->get_ask_user_to_redirect());
	maxRedirectTextLabel->setEnabled(current_profile->get_allow_redirection());
	maxRedirectSpinBox->setEnabled(current_profile->get_allow_redirection());
	maxRedirectSpinBox->setValue(current_profile->get_max_redirections());
	ext100relComboBox->setCurrentItem(
			ext_support2indexComboItem(current_profile->get_ext_100rel()));
	extReplacesCheckBox->setChecked(current_profile->get_ext_replaces());
	allowReferCheckBox->setChecked(current_profile->get_allow_refer());
	askUserReferCheckBox->setEnabled(current_profile->get_allow_refer());
	askUserReferCheckBox->setChecked(current_profile->get_ask_user_to_refer());
	refereeHoldCheckBox->setEnabled(current_profile->get_allow_refer());
	refereeHoldCheckBox->setChecked(current_profile->get_referee_hold());
	referrerHoldCheckBox->setChecked(current_profile->get_referrer_hold());
	refreshReferSubCheckBox->setChecked(current_profile->get_auto_refresh_refer_sub());
	referAorCheckBox->setChecked(current_profile->get_attended_refer_to_aor());
	transferConsultInprogCheckBox->setChecked(
			current_profile->get_allow_transfer_consultation_inprog());
	pPreferredIdCheckBox->setChecked(current_profile->get_send_p_preferred_id());
	
	// Transport/NAT
	switch (current_profile->get_sip_transport()) {
	case SIP_TRANS_UDP:
		sipTransportComboBox->setCurrentItem(idxSipTransportUDP);
		break;
	case SIP_TRANS_TCP:
		sipTransportComboBox->setCurrentItem(idxSipTransportTCP);
		break;
	default:
		sipTransportComboBox->setCurrentItem(idxSipTransportAuto);
		break;
	}
	
	udpThresholdSpinBox->setValue(current_profile->get_sip_transport_udp_threshold());
	udpThresholdTextLabel->setEnabled(current_profile->get_sip_transport() == SIP_TRANS_AUTO);
	udpThresholdSpinBox->setEnabled(current_profile->get_sip_transport() == SIP_TRANS_AUTO);
	
	if (current_profile->get_use_nat_public_ip()) {
		natStaticRadioButton->setChecked(true);
	} else if (current_profile->get_use_stun()) {
		natStunRadioButton->setChecked(true);
	} else {
		natNoneRadioButton->setChecked(true);
	}
	
	publicIPTextLabel->setEnabled(current_profile->get_use_nat_public_ip());
	publicIPLineEdit->setEnabled(current_profile->get_use_nat_public_ip());
	publicIPLineEdit->setText(current_profile->get_nat_public_ip().c_str());
	stunServerTextLabel->setEnabled(current_profile->get_use_stun());
	stunServerLineEdit->setEnabled(current_profile->get_use_stun());
	stunServerLineEdit->setText(current_profile->get_stun_server().
				    encode_noscheme().c_str());
	persistentTcpCheckBox->setChecked(current_profile->get_persistent_tcp());
	persistentTcpCheckBox->setEnabled(current_profile->get_sip_transport() == SIP_TRANS_TCP);
	natKeepaliveCheckBox->setChecked(current_profile->get_enable_nat_keepalive());
	natKeepaliveCheckBox->setDisabled(current_profile->get_use_stun());
	
	// ADDRESS FORMAT
	displayTelUserCheckBox->setChecked(current_profile->get_display_useronly_phone());
	numericalUserIsTelCheckBox->setChecked(
			current_profile->get_numerical_user_is_phone());
	removeSpecialCheckBox->setChecked(
			current_profile->get_remove_special_phone_symbols());
	specialLineEdit->setText(current_profile->get_special_phone_symbols().c_str());
	useTelUriCheckBox->setChecked(current_profile->get_use_tel_uri_for_phone());
	
	conversionListView->clear();
	conversionListView->setSorting(-1);
	list<t_number_conversion> conversions = current_profile->get_number_conversions();
	for (list<t_number_conversion>::reverse_iterator i = conversions.rbegin(); i != conversions.rend(); i++)
	{
		new QListViewItem(conversionListView, i->re.str().c_str(), i->fmt.c_str());
	}
	
	// TIMERS
	tmrNoanswerSpinBox->setValue(current_profile->get_timer_noanswer());
	tmrNatKeepaliveSpinBox->setValue(current_profile->get_timer_nat_keepalive());
	
	// RING TONES
	ringtoneLineEdit->setText(current_profile->get_ringtone_file().c_str());
	ringbackLineEdit->setText(current_profile->get_ringback_file().c_str());
	
	// SCRIPTS
	incomingCallScriptLineEdit->setText(current_profile->get_script_incoming_call().c_str());
	inCallAnsweredLineEdit->setText(current_profile->get_script_in_call_answered().c_str());
	inCallFailedLineEdit->setText(current_profile->get_script_in_call_failed().c_str());
	outCallLineEdit->setText(current_profile->get_script_outgoing_call().c_str());
	outCallAnsweredLineEdit->setText(current_profile->get_script_out_call_answered().c_str());
	outCallFailedLineEdit->setText(current_profile->get_script_out_call_failed().c_str());
	localReleaseLineEdit->setText(current_profile->get_script_local_release().c_str());
	remoteReleaseLineEdit->setText(current_profile->get_script_remote_release().c_str());
	
	// Security
	zrtpEnabledCheckBox->setChecked(current_profile->get_zrtp_enabled());
	zrtpSettingsGroupBox->setEnabled(current_profile->get_zrtp_enabled());
	zrtpSendIfSupportedCheckBox->setChecked(current_profile->get_zrtp_send_if_supported());
	zrtpSdpCheckBox->setChecked(current_profile->get_zrtp_sdp());
	zrtpGoClearWarningCheckBox->setChecked(current_profile->get_zrtp_goclear_warning());
}

void UserProfileForm::initProfileList(list<t_user *> profiles, QString show_profile_name)
{
	profile_list = profiles;
	
	// Initialize user profile combo box
	current_profile_idx = -1;
	profileComboBox->clear();
	
	t_user *show_profile = NULL;
	int show_idx = 0;
	int idx = 0;
	for (list<t_user *>::iterator i = profile_list.begin(); i != profile_list.end(); i++) {
		profileComboBox->insertItem((*i)->get_profile_name().c_str());
		if (show_profile_name == (*i)->get_profile_name().c_str()) {
			show_idx = idx;
			show_profile = *i;
		}
		idx++;
	}
	
	profileComboBox->setEnabled(profile_list.size() > 1);
	current_profile_idx = show_idx;
	
	if (show_profile == NULL) {
		current_profile = profile_list.front();
	} else {
		current_profile = show_profile;
	}
	profileComboBox->setCurrentItem(current_profile_idx);
}

// Show the form
void UserProfileForm::show(list<t_user *> profiles, QString show_profile)
{
	map_last_cat.clear();
	initProfileList(profiles, show_profile);
	populate();
	
	// Show form
	QDialog::show();
}

// Modal execution
int UserProfileForm::exec(list<t_user *> profiles, QString show_profile)
{
	map_last_cat.clear();
	initProfileList(profiles, show_profile);
	populate();
	return QDialog::exec();
}

bool UserProfileForm::check_dynamic_payload(QSpinBox *spb, 
					    QValueList<int> &checked_list) 
{
	if (checked_list.contains(spb->value())) {
		categoryListBox->setSelected(idxCatRtpAudio, true);
		settingsWidgetStack->raiseWidget(pageRtpAudio);
		QString msg = tr("Dynamic payload type %1 is used more than once.").arg(spb->value());
		((t_gui *)ui)->cb_show_msg(this, msg.ascii(), MSG_CRITICAL);
		spb->setFocus();
		return false;
	}
	
	checked_list.append(spb->value());
	return true;
}

list<t_number_conversion> UserProfileForm::get_number_conversions()
{
	list<t_number_conversion> conversions;
	QListViewItemIterator it(conversionListView);
	while (it.current()) {
		QListViewItem *item = it.current();
		t_number_conversion c;
		
		try {
			c.re.assign(item->text(colExpr).ascii());
			c.fmt = item->text(colReplace).ascii();
			conversions.push_back(c);
		} catch (boost::bad_expression) {
			// Should never happen as validity has been
			// checked already. Just being defensive here.
		}

		++it;
	}
	
	return conversions;
}
	    
bool UserProfileForm::validateValues()
{
	QString s;
	
	// Validity check user page
	// SIP username is mandatory
	if (usernameLineEdit->text().isEmpty()) {
		categoryListBox->setSelected(idxCatUser, true);
		settingsWidgetStack->raiseWidget(pageUser);
		((t_gui *)ui)->cb_show_msg(this, tr("You must fill in a user name for your SIP account.").ascii(),
				MSG_CRITICAL);
		usernameLineEdit->setFocus();
		return false;
	}
	
	// SIP user domain is mandatory
	if (domainLineEdit->text().isEmpty()) {
		categoryListBox->setSelected(idxCatUser, true);
		settingsWidgetStack->raiseWidget(pageUser);
		((t_gui *)ui)->cb_show_msg(this, tr(
				"You must fill in a domain name for your SIP account.\n"
				"This could be the hostname or IP address of your PC "
				"if you want direct PC to PC dialing.").ascii(),
				MSG_CRITICAL);
		domainLineEdit->setFocus();
		return false;
	}
	
	// Check validity of domain
	s = USER_SCHEME;
	s.append(':').append(domainLineEdit->text());
	t_url u_domain(s.ascii());
	if (!u_domain.is_valid() || u_domain.get_user() != "") {
		categoryListBox->setSelected(idxCatUser, true);
		settingsWidgetStack->raiseWidget(pageUser);
		((t_gui *)ui)->cb_show_msg(this,  tr("Invalid domain.").ascii(), MSG_CRITICAL);
		domainLineEdit->setFocus();
		return false;
	}
	
	// Check validity of user
	s = USER_SCHEME;
	s.append(':').append(usernameLineEdit->text()).append('@');
	s.append(domainLineEdit->text());
	t_url u_user_domain(s.ascii());
	if (!u_user_domain.is_valid()) {
		categoryListBox->setSelected(idxCatUser, true);
		settingsWidgetStack->raiseWidget(pageUser);
		((t_gui *)ui)->cb_show_msg(this,  tr("Invalid user name.").ascii(), MSG_CRITICAL);
		usernameLineEdit->setFocus();
		return false;
	}
	
	// Registrar
	if (!registrarLineEdit->text().isEmpty()) {
		s = USER_SCHEME;
		s.append(':').append(registrarLineEdit->text());
		t_url u(s.ascii());
		if (!u.is_valid() || u.get_user() != "") {
			categoryListBox->setSelected(idxCatSipServer, true);
			settingsWidgetStack->raiseWidget(pageSipServer);
			((t_gui *)ui)->cb_show_msg(this, tr("Invalid value for registrar.").ascii(), 
						   MSG_CRITICAL);
			registrarLineEdit->setFocus();
			registrarLineEdit->selectAll();
			return false;
		}
	}
	
	// Outbound proxy
	if (useProxyCheckBox->isChecked()) {
		s = USER_SCHEME;
		s.append(':').append(proxyLineEdit->text());
		t_url u(s.ascii());
		if (!u.is_valid() || u.get_user() != "") {
			categoryListBox->setSelected(idxCatSipServer, true);
			settingsWidgetStack->raiseWidget(pageSipServer);
			((t_gui *)ui)->cb_show_msg(this, tr("Invalid value for outbound proxy.").ascii(), 
					MSG_CRITICAL);
			proxyLineEdit->setFocus();
			proxyLineEdit->selectAll();
			return false;
		}
	}
	

	// Validity check voice mail page
	if (mwiTypeComboBox->currentItem() == idxMWISollicited) {
		// Mailbox user name is mandatory
		if (mwiUserLineEdit->text().isEmpty()) {
			categoryListBox->setSelected(idxCatVoiceMail, true);
			settingsWidgetStack->raiseWidget(pageVoiceMail);
			((t_gui *)ui)->cb_show_msg(this, 
					tr("You must fill in a mailbox user name.").ascii(),
					MSG_CRITICAL);
			mwiUserLineEdit->setFocus();
			return false;
		}
		
		// Mailbox server is mandatory
		if (mwiServerLineEdit->text().isEmpty()) {
			categoryListBox->setSelected(idxCatVoiceMail, true);
			settingsWidgetStack->raiseWidget(pageVoiceMail);
			((t_gui *)ui)->cb_show_msg(this, 
					tr("You must fill in a mailbox server").ascii(),
					MSG_CRITICAL);
			mwiServerLineEdit->setFocus();
			return false;
		}
		
		// Check validity of mailbox server
		s = USER_SCHEME;
		s.append(':').append(mwiServerLineEdit->text());
		t_url u_server(s.ascii());
		if (!u_server.is_valid() || u_server.get_user() != "") {
			categoryListBox->setSelected(idxCatVoiceMail, true);
			settingsWidgetStack->raiseWidget(pageVoiceMail);
			((t_gui *)ui)->cb_show_msg(this,  tr("Invalid mailbox server.").ascii(), 
						   MSG_CRITICAL);
			mwiServerLineEdit->setFocus();
			return false;
		}
		
		// Check validity of mailbox user name
		s = USER_SCHEME;
		s.append(':').append(mwiUserLineEdit->text()).append('@');
		s.append(mwiServerLineEdit->text());
		t_url u_user_server(s.ascii());
		if (!u_user_server.is_valid()) {
			categoryListBox->setSelected(idxCatVoiceMail, true);
			settingsWidgetStack->raiseWidget(pageVoiceMail);
			((t_gui *)ui)->cb_show_msg(this,  tr("Invalid mailbox user name.").ascii(), 
						   MSG_CRITICAL);
			mwiUserLineEdit->setFocus();
			return false;
		}
	}
	
	// NAT public IP
	if (natStaticRadioButton->isChecked()) {
		if (publicIPLineEdit->text().isEmpty()){
			categoryListBox->setSelected(idxCatNat, true);
			settingsWidgetStack->raiseWidget(pageNat);
			((t_gui *)ui)->cb_show_msg(this, tr("Value for public IP address missing.").ascii(),
					MSG_CRITICAL);
			publicIPLineEdit->setFocus();
			return false;
		}
	}
	
	// Check for double RTP dynamic payload types
	QValueList<int> checked_types;
	if (!check_dynamic_payload(spxNbPayloadSpinBox, checked_types) ||
	    !check_dynamic_payload(spxWbPayloadSpinBox, checked_types) ||
	    !check_dynamic_payload(spxUwbPayloadSpinBox, checked_types))
	{
		rtpAudioTabWidget->showPage(tabSpeex);
		return false;
	}
	
	if (!check_dynamic_payload(ilbcPayloadSpinBox, checked_types)) {
		rtpAudioTabWidget->showPage(tabIlbc);
		return false;
	}
	
	if (!check_dynamic_payload(g72616PayloadSpinBox, checked_types) ||
	    !check_dynamic_payload(g72624PayloadSpinBox, checked_types) ||
	    !check_dynamic_payload(g72632PayloadSpinBox, checked_types) ||
	    !check_dynamic_payload(g72640PayloadSpinBox, checked_types)) {
		rtpAudioTabWidget->showPage(tabG726);
		return false;
	}
	
	if (!check_dynamic_payload(dtmfPayloadTypeSpinBox, checked_types)) {
		rtpAudioTabWidget->showPage(tabDtmf);
		return false;
	}
	
	// STUN server
	if (natStunRadioButton->isChecked()) {
		s = "stun:";
		s.append(stunServerLineEdit->text());
		t_url u(s.ascii());
		if (!u.is_valid() || u.get_user() != "") {
			categoryListBox->setSelected(idxCatNat, true);
			settingsWidgetStack->raiseWidget(pageNat);
			((t_gui *)ui)->cb_show_msg(this, tr("Invalid value for STUN server.").ascii(), 
					MSG_CRITICAL);
			stunServerLineEdit->setFocus();
			stunServerLineEdit->selectAll();
			return false;
		}
	}
	
	// Clear outbound proxy if not used
	if (!useProxyCheckBox->isChecked()) {
		proxyLineEdit->clear();
	}
	
	// Clear sollicited MWI settings if unsollicited MWI is used
	if (mwiTypeComboBox->currentItem() == idxMWIUnsollicited) {
		t_user user_default;
		mwiUserLineEdit->clear();
		mwiServerLineEdit->clear();
		mwiViaProxyCheckBox->setChecked(user_default.get_mwi_via_proxy());
		mwiDurationSpinBox->setValue(user_default.get_mwi_subscription_time());
	}
	
	// Clear NAT public IP if not used
	if (!natStaticRadioButton->isChecked()) {
		publicIPLineEdit->clear();
	}
	
	// Clear STUN server if not used
	if (!natStunRadioButton->isChecked()) {
		stunServerLineEdit->clear();
	}
	
	// Set all values in the current_profile object
	// USER
	if (current_profile->get_name() != usernameLineEdit->text().ascii() ||
	    current_profile->get_display(false) != displayLineEdit->text().ascii() ||
	    current_profile->get_domain() != domainLineEdit->text().ascii())
	{
		current_profile->set_display(displayLineEdit->text().ascii());
		current_profile->set_name(usernameLineEdit->text().ascii());
		current_profile->set_domain (domainLineEdit->text().ascii());
		emit sipUserChanged(current_profile);
	}
	
	current_profile->set_organization(organizationLineEdit->text().ascii());
	
	uint8 new_aka_op[AKA_OPLEN];
	uint8 new_aka_amf[AKA_AMFLEN];
	uint8 current_aka_op[AKA_OPLEN];
	uint8 current_aka_amf[AKA_AMFLEN];
	
	hex2binary(padleft(authAkaOpLineEdit->text().ascii(), '0', 32), new_aka_op);
	hex2binary(padleft(authAkaAmfLineEdit->text().ascii(), '0', 4), new_aka_amf);
	current_profile->get_auth_aka_op(current_aka_op);
	current_profile->get_auth_aka_amf(current_aka_amf);
	
	if (current_profile->get_auth_realm() != authRealmLineEdit->text().ascii() ||
	    current_profile->get_auth_name() != authNameLineEdit->text().ascii() ||
	    current_profile->get_auth_pass() != authPasswordLineEdit->text().ascii() ||
	    memcmp(current_aka_op, new_aka_op, AKA_OPLEN) != 0 ||
	    memcmp(current_aka_amf, new_aka_amf, AKA_AMFLEN) != 0)
	{
		emit authCredentialsChanged(current_profile,
					current_profile->get_auth_realm());
		
		current_profile->set_auth_realm(authRealmLineEdit->text().ascii());
		current_profile->set_auth_name(authNameLineEdit->text().ascii());
		current_profile->set_auth_pass(authPasswordLineEdit->text().ascii());
		current_profile->set_auth_aka_op(new_aka_op);
		current_profile->set_auth_aka_amf(new_aka_amf);
	}

	// SIP SERVER
	current_profile->set_use_registrar(!registrarLineEdit->text().isEmpty());
	s = USER_SCHEME;
	s.append(':').append(registrarLineEdit->text());
	current_profile->set_registrar(t_url(s.ascii()));
	current_profile->set_registration_time(expirySpinBox->value());
	current_profile->set_register_at_startup(regAtStartupCheckBox->isChecked());
	current_profile->set_reg_add_qvalue(regAddQvalueCheckBox->isChecked());
	current_profile->set_reg_qvalue(atof(regQvalueLineEdit->text().ascii()));
	
	current_profile->set_use_outbound_proxy(useProxyCheckBox->isChecked());
	s = USER_SCHEME;
	s.append(':').append(proxyLineEdit->text());
	current_profile->set_outbound_proxy(t_url(s.ascii()));
	current_profile->set_all_requests_to_proxy(allRequestsCheckBox->isChecked());
	current_profile->set_non_resolvable_to_proxy(
			proxyNonResolvableCheckBox->isChecked());
	
	// VOICE MAIL
	current_profile->set_mwi_vm_address(vmAddressLineEdit->text().ascii());
	
	bool mustTriggerMWISubscribe = false;
	bool mwiSollicited = (mwiTypeComboBox->currentItem() == idxMWISollicited);
	if (mwiSollicited) {
		if (!current_profile->get_mwi_sollicited()) {
			// Sollicited MWI now enabled. Subscribe after all MWI
			// settings have been changed.
			mustTriggerMWISubscribe = true;
		} else {
			s = USER_SCHEME;
			s.append(':').append(mwiServerLineEdit->text());
			if (mwiUserLineEdit->text().ascii() != current_profile->get_mwi_user() ||
			    t_url(s.ascii()) != current_profile->get_mwi_server() ||
			    mwiViaProxyCheckBox->isChecked() != current_profile->get_mwi_via_proxy())
			{
				// Sollicited MWI settings changed. Trigger unsubscribe
				// of current MWI subscription.
				emit mwiChangeUnsubscribe(current_profile);
				
				// Subscribe after the settings have been changed.
				mustTriggerMWISubscribe = true;
			}
		}
	} else {
		if (current_profile->get_mwi_sollicited()) {
			// MWI type changes to unsollicited. Trigger unsubscribe of
			// current MWI subscription.
			emit mwiChangeUnsubscribe(current_profile);
		}
	}
	
	current_profile->set_mwi_sollicited(mwiSollicited);
	current_profile->set_mwi_user(mwiUserLineEdit->text().ascii());
	s = USER_SCHEME;
	s.append(':').append(mwiServerLineEdit->text());
	current_profile->set_mwi_server(t_url(s.ascii()));
	current_profile->set_mwi_via_proxy(mwiViaProxyCheckBox->isChecked());
	current_profile->set_mwi_subscription_time(mwiDurationSpinBox->value());
	
	if (mustTriggerMWISubscribe) {
		emit mwiChangeSubscribe(current_profile);
	}
	
	// INSTANT MESSAGE
	current_profile->set_im_max_sessions(imMaxSessionsSpinBox->value());
	current_profile->set_im_send_iscomposing(isComposingCheckBox->isChecked());
	
	// PRESENCE
	current_profile->set_pres_publish_startup(presPublishCheckBox->isChecked());
	current_profile->set_pres_publication_time(presPublishTimeSpinBox->value());
	current_profile->set_pres_subscription_time(presSubscribeTimeSpinBox->value());
	
	// RTP AUDIO
	// Codecs
	list<t_audio_codec> audio_codecs;
	for (size_t i = 0; i < activeCodecListBox->count(); i++) {
		audio_codecs.push_back(label2codec(activeCodecListBox->text(i)));
	}
	current_profile->set_codecs(audio_codecs);
	
	// G.711/G.726 ptime
	current_profile->set_ptime(ptimeSpinBox->value());
	
	// Codec preference
	current_profile->set_in_obey_far_end_codec_pref(inFarEndCodecPrefCheckBox->isChecked());
	current_profile->set_out_obey_far_end_codec_pref(outFarEndCodecPrefCheckBox->isChecked());
	
	// Speex preprocessing & AEC
 	current_profile->set_speex_dsp_vad(spxDspVadCheckBox->isChecked());
	current_profile->set_speex_dsp_agc(spxDspAgcCheckBox->isChecked());
	current_profile->set_speex_dsp_aec(spxDspAecCheckBox->isChecked());
	current_profile->set_speex_dsp_nrd(spxDspNrdCheckBox->isChecked());
	current_profile->set_speex_dsp_agc_level(spxDspAgcLevelSpinBox->value());

	// Speex ([en/de]coding)
	current_profile->set_speex_bit_rate_type((spxVbrCheckBox->isChecked() ? BIT_RATE_VBR : BIT_RATE_CBR));
	current_profile->set_speex_dtx(spxDtxCheckBox->isChecked());
	current_profile->set_speex_penh(spxPenhCheckBox->isChecked());
	current_profile->set_speex_quality(spxQualitySpinBox->value());
	current_profile->set_speex_complexity(spxComplexitySpinBox->value());
	current_profile->set_speex_nb_payload_type(spxNbPayloadSpinBox->value());
	current_profile->set_speex_wb_payload_type(spxWbPayloadSpinBox->value());
	current_profile->set_speex_uwb_payload_type(spxUwbPayloadSpinBox->value());
	
	// iLBC
	current_profile->set_ilbc_payload_type(ilbcPayloadSpinBox->value());
	switch (ilbcPayloadSizeComboBox->currentItem()) {
	case idxIlbcMode20:
		current_profile->set_ilbc_mode(20);
		break;
	default:
		current_profile->set_ilbc_mode(30);
		break;
	}
	
	// G726
	current_profile->set_g726_16_payload_type(g72616PayloadSpinBox->value());
	current_profile->set_g726_24_payload_type(g72624PayloadSpinBox->value());
	current_profile->set_g726_32_payload_type(g72632PayloadSpinBox->value());
	current_profile->set_g726_40_payload_type(g72640PayloadSpinBox->value());
	
	switch (g726PackComboBox->currentItem()) {
	case idxG726PackRfc3551:
		current_profile->set_g726_packing(G726_PACK_RFC3551);
		break;
	default:
		current_profile->set_g726_packing(G726_PACK_AAL2);
		break;
	}
	
	// DTMF
	switch (dtmfTransportComboBox->currentItem()) {
	case idxDtmfRfc2833:
		current_profile->set_dtmf_transport(DTMF_RFC2833);
		break;
	case idxDtmfInband:
		current_profile->set_dtmf_transport(DTMF_INBAND);
		break;
	case idxDtmfInfo:
		current_profile->set_dtmf_transport(DTMF_INFO);
		break;
	default:
		current_profile->set_dtmf_transport(DTMF_AUTO);
		break;
	}
	
	current_profile->set_dtmf_payload_type(dtmfPayloadTypeSpinBox->value());
	current_profile->set_dtmf_duration(dtmfDurationSpinBox->value());
	current_profile->set_dtmf_pause(dtmfPauseSpinBox->value());
	current_profile->set_dtmf_volume(-(dtmfVolumeSpinBox->value()));
	
	// SIP PROTOCOL
	switch (holdVariantComboBox->currentItem()) {
	case idxHoldRfc2543:
		current_profile->set_hold_variant(HOLD_RFC2543);
		break;
	default:
		current_profile->set_hold_variant(HOLD_RFC3264);
		break;
	}
	
	current_profile->set_check_max_forwards(maxForwardsCheckBox->isChecked());
	current_profile->set_allow_missing_contact_reg(missingContactCheckBox->isChecked());
	current_profile->set_registration_time_in_contact(regTimeCheckBox->isChecked());
	current_profile->set_compact_headers(compactHeadersCheckBox->isChecked());
	current_profile->set_encode_multi_values_as_list(
			multiValuesListCheckBox->isChecked());
	current_profile->set_use_domain_in_contact(
			useDomainInContactCheckBox->isChecked());
	current_profile->set_allow_sdp_change(allowSdpChangeCheckBox->isChecked());
	current_profile->set_allow_redirection(allowRedirectionCheckBox->isChecked());
	current_profile->set_ask_user_to_redirect(askUserRedirectCheckBox->isChecked());
	current_profile->set_max_redirections(maxRedirectSpinBox->value());
	current_profile->set_ext_100rel(indexComboItem2ext_support(
			ext100relComboBox->currentItem()));
	current_profile->set_ext_replaces(extReplacesCheckBox->isChecked());
	current_profile->set_allow_refer(allowReferCheckBox->isChecked());
	current_profile->set_ask_user_to_refer(askUserReferCheckBox->isChecked());
	current_profile->set_referee_hold(refereeHoldCheckBox->isChecked());
	current_profile->set_referrer_hold(referrerHoldCheckBox->isChecked());
	current_profile->set_auto_refresh_refer_sub(refreshReferSubCheckBox->isChecked());
	current_profile->set_attended_refer_to_aor(referAorCheckBox->isChecked());
	current_profile->set_allow_transfer_consultation_inprog(
			transferConsultInprogCheckBox->isChecked());
	current_profile->set_send_p_preferred_id(pPreferredIdCheckBox->isChecked());
	
	// Transport/NAT
	switch (sipTransportComboBox->currentItem()) {
	case idxSipTransportUDP:
		current_profile->set_sip_transport(SIP_TRANS_UDP);
		break;
	case idxSipTransportTCP:
		current_profile->set_sip_transport(SIP_TRANS_TCP);
		break;
	default:
		current_profile->set_sip_transport(SIP_TRANS_AUTO);
		break;
	}
	
	current_profile->set_sip_transport_udp_threshold(udpThresholdSpinBox->value());
	
	current_profile->set_use_nat_public_ip(natStaticRadioButton->isChecked());
	current_profile->set_nat_public_ip(publicIPLineEdit->text().ascii());
	current_profile->set_use_stun(natStunRadioButton->isChecked());
	
	if (current_profile->get_stun_server().encode_noscheme() != stunServerLineEdit->text().ascii() ||
	    current_profile->get_enable_nat_keepalive() != natKeepaliveCheckBox->isChecked()) 
	{
		s = "stun:";
		s.append(stunServerLineEdit->text());
		current_profile->set_stun_server(t_url(s.ascii()));
		current_profile->set_enable_nat_keepalive(natKeepaliveCheckBox->isChecked());
		emit stunServerChanged(current_profile);
	}
	
	current_profile->set_persistent_tcp(persistentTcpCheckBox->isChecked());
	
	// ADDRESS FORMAT
	current_profile->set_display_useronly_phone(
			displayTelUserCheckBox->isChecked());
	current_profile->set_numerical_user_is_phone(
			numericalUserIsTelCheckBox->isChecked());
	current_profile->set_remove_special_phone_symbols(
			removeSpecialCheckBox->isChecked());
	current_profile->set_special_phone_symbols(
			specialLineEdit->text().stripWhiteSpace().ascii());
	current_profile->set_number_conversions(get_number_conversions());
	current_profile->set_use_tel_uri_for_phone(useTelUriCheckBox->isChecked());
	
	// TIMERS
	current_profile->set_timer_noanswer(tmrNoanswerSpinBox->value());
	current_profile->set_timer_nat_keepalive(tmrNatKeepaliveSpinBox->value());
	
	// RING TONES
	current_profile->set_ringtone_file(ringtoneLineEdit->text().stripWhiteSpace().ascii());
	current_profile->set_ringback_file(ringbackLineEdit->text().stripWhiteSpace().ascii());
	
	// SCRIPTS
	current_profile->set_script_incoming_call(incomingCallScriptLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_in_call_answered(inCallAnsweredLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_in_call_failed(inCallFailedLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_outgoing_call(outCallLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_out_call_answered(outCallAnsweredLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_out_call_failed(outCallFailedLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_local_release(localReleaseLineEdit->
					text().stripWhiteSpace().ascii());
	current_profile->set_script_remote_release(remoteReleaseLineEdit->
					text().stripWhiteSpace().ascii());
	
	// Security
	current_profile->set_zrtp_enabled(zrtpEnabledCheckBox->isChecked());
	current_profile->set_zrtp_send_if_supported(zrtpSendIfSupportedCheckBox->isChecked());
	current_profile->set_zrtp_sdp(zrtpSdpCheckBox->isChecked());
	current_profile->set_zrtp_goclear_warning(zrtpGoClearWarningCheckBox->isChecked());
	
	// Save user config
	string error_msg;
	if (!current_profile->write_config(current_profile->get_filename(), error_msg)) {
		// Failed to write config file
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_CRITICAL);
		return false;
	}
	
	return true;
}

void UserProfileForm::validate() {
	if (validateValues()) {
		emit success();
		accept();
	}
}

// User wants to change to another profile
void UserProfileForm::changeProfile(const QString &profileName) {
	if (current_profile_idx == -1) {
		// Initializing combo box
		return;
	}
	
	// Make the current profile permanent.
	if (!validateValues()) {
		// Current values are not valid.
		// Do not change to the new profile.
		profileComboBox->setCurrentItem(current_profile_idx);
		return;
	}
	
	// Store the current viewed category
	map_last_cat[current_profile] = categoryListBox->index(categoryListBox->selectedItem());
	
	// Change to new profile.
	for (list<t_user *>::iterator i = profile_list.begin(); i != profile_list.end(); i++) {
		if ((*i)->get_profile_name() == profileName.ascii()) {
			current_profile = *i;
			break;
		}
	}
	
	current_profile_idx = profileComboBox->currentItem();
	populate();
	
	// Restore last viewed category
	int idxCat = map_last_cat[current_profile];
	categoryListBox->setSelected(idxCat, true);
	showCategory(idxCat);
}

void UserProfileForm::chooseFile(QLineEdit *qle, const QString &filter, const QString &caption) 
{
	QString file = QFileDialog::getOpenFileName(
			((t_gui *)ui)->get_last_file_browse_path(),
			filter, this, "open file dialog",
			caption);
	if (!file.isEmpty()) {
		qle->setText(file);
		((t_gui *)ui)->set_last_file_browse_path(QFileInfo(file).dirPath(true));
	}	
}

void UserProfileForm::chooseRingtone()
{
	chooseFile(ringtoneLineEdit, tr("Ring tones", "Description of .wav files in file dialog").append(" (*.wav)"), tr("Choose ring tone"));
}

void UserProfileForm::chooseRingback()
{
	chooseFile(ringbackLineEdit, tr("Ring back tones", "Description of .wav files in file dialog").append(" (*.wav)"), "Choose ring back tone");
}

void UserProfileForm::chooseIncomingCallScript()
{
	chooseFile(incomingCallScriptLineEdit, tr("All files").append(" (*)"), tr("Choose incoming call script"));
}

void UserProfileForm::chooseInCallAnsweredScript()
{
	chooseFile(inCallAnsweredLineEdit, tr("All files").append(" (*)"), tr("Choose incoming call answered script"));
}

void UserProfileForm::chooseInCallFailedScript()
{
	chooseFile(inCallFailedLineEdit, tr("All files").append(" (*)"), tr("Choose incoming call failed script"));
}

void UserProfileForm::chooseOutgoingCallScript()
{
	chooseFile(outCallLineEdit, tr("All files").append(" (*)"), tr("Choose outgoing call script"));
}

void UserProfileForm::chooseOutCallAnsweredScript()
{
	chooseFile(outCallAnsweredLineEdit, tr("All files").append(" (*)"), tr("Choose outgoing call answered script"));
}

void UserProfileForm::chooseOutCallFailedScript()
{
	chooseFile(outCallFailedLineEdit, tr("All files").append(" (*)"), tr("Choose outgoing call failed script"));
}

void UserProfileForm::chooseLocalReleaseScript()
{
	chooseFile(localReleaseLineEdit, tr("All files").append(" (*)"), tr("Choose local release script"));
}

void UserProfileForm::chooseRemoteReleaseScript()
{
	chooseFile(remoteReleaseLineEdit, tr("All files").append(" (*)"), tr("Choose remote release script"));
}

void UserProfileForm::addCodec() {
	for (size_t i = 0; i < availCodecListBox->count(); i++) {
		if (availCodecListBox->isSelected(i)) {
			activeCodecListBox->insertItem(availCodecListBox->text(i));
			activeCodecListBox->setSelected(
					activeCodecListBox->count() - 1, true);
			availCodecListBox->removeItem(i);
			return;
		}
	}
}

void UserProfileForm::removeCodec() {
	for (size_t i = 0; i < activeCodecListBox->count(); i++) {
		if (activeCodecListBox->isSelected(i)) {
			availCodecListBox->insertItem(activeCodecListBox->text(i));
			availCodecListBox->setSelected(
					availCodecListBox->count() - 1, true);
			activeCodecListBox->removeItem(i);
			return;
		}
	}
}

void UserProfileForm::upCodec() {
	QListBoxItem *lbi = activeCodecListBox->selectedItem();
	if (!lbi) return;
	
	int idx = activeCodecListBox->index(lbi);
	if (idx == 0) return;
	
	QString label = lbi->text();
	activeCodecListBox->removeItem(idx);
	activeCodecListBox->insertItem(label, idx - 1);
	activeCodecListBox->setSelected(idx - 1, true);
}

void UserProfileForm::downCodec() {
	QListBoxItem *lbi = activeCodecListBox->selectedItem();
	if (!lbi) return;
	
	size_t idx = activeCodecListBox->index(lbi);
	if (idx == activeCodecListBox->count() - 1) return;
	
	QString label = lbi->text();
	activeCodecListBox->removeItem(idx);
	activeCodecListBox->insertItem(label, idx + 1);
	activeCodecListBox->setSelected(idx + 1, true);
}

void UserProfileForm::upConversion() {
	QListViewItem *lvi = conversionListView->selectedItem();
	if (!lvi) return;
	
	QListViewItem *above = lvi->itemAbove();
	if (!above) return;
	
	QListViewItem *newAbove = above->itemAbove();
	
	if (newAbove) {
		lvi->moveItem(newAbove);
	} else {
		above->moveItem(lvi);
	}
		
	lvi->setSelected(true);
}

void UserProfileForm::downConversion() {
	QListViewItem *lvi = conversionListView->selectedItem();
	if (!lvi) return;
	
	QListViewItem *below = lvi->itemBelow();
	if (!below) return;
	
	lvi->moveItem(below);
	lvi->setSelected(true);
}

void UserProfileForm::addConversion() {
	QString expr;
	QString replace;
	
	NumberConversionForm f;
	if (f.exec(expr, replace) == QDialog::Accepted) {
		QListViewItem *last = conversionListView->lastItem();
		if (last) {
			new QListViewItem(conversionListView, last, expr, replace);
		} else {
			new QListViewItem(conversionListView, expr, replace);
		}
	}
}

void UserProfileForm::editConversion() {
	QListViewItem *lvi = conversionListView->selectedItem();
	if (!lvi) return;
	 
	QString expr = lvi->text(colExpr);
	QString replace = lvi->text(colReplace);
	
	NumberConversionForm f;
	if (f.exec(expr, replace) == QDialog::Accepted) {
		lvi->setText(colExpr, expr);
		lvi->setText(colReplace, replace);
	}
}

void UserProfileForm::removeConversion() {
	QListViewItem *lvi = conversionListView->selectedItem();
	if (!lvi) return;
	delete lvi;
}

void UserProfileForm::testConversion() {
	QString number = testConversionLineEdit->text();
	if (number.isEmpty()) return;
	
	bool remove_special_phone_symbols = removeSpecialCheckBox->isChecked();
	QString special_phone_symbols = specialLineEdit->text();
	
	number = remove_white_space(number.ascii()).c_str();
	
	// Remove special symbols
	if (remove_special_phone_symbols &&
	    looks_like_phone(number.ascii(), special_phone_symbols.ascii()))
	{
		number = remove_symbols(
				number.ascii(), special_phone_symbols.ascii()).c_str();
	}
	
	QString msg = tr("%1 converts to %2")
		      .arg(number)
		      .arg(current_profile->convert_number(number.ascii(), get_number_conversions()).c_str());
	
	((t_gui *)ui)->cb_show_msg(this,  msg.ascii(), MSG_INFO);
}

void UserProfileForm::changeMWIType(int idxMWIType) {
	if (idxMWIType == idxMWISollicited) {
		mwiSollicitedGroupBox->setEnabled(true);
		
		// Set defaults
		if (mwiUserLineEdit->text().isEmpty()) {
			mwiUserLineEdit->setText(usernameLineEdit->text());
		}
		if (mwiServerLineEdit->text().isEmpty()) {
			mwiServerLineEdit->setText(domainLineEdit->text());
			mwiViaProxyCheckBox->setChecked(useProxyCheckBox->isChecked());
		}
	} else {
		mwiSollicitedGroupBox->setEnabled(false);
	}
}

void UserProfileForm::changeSipTransportProtocol(int idx) {
	udpThresholdTextLabel->setEnabled(idx == idxSipTransportAuto);
	udpThresholdSpinBox->setEnabled(idx == idxSipTransportAuto);
	persistentTcpCheckBox->setEnabled(idx == idxSipTransportTCP);
}
