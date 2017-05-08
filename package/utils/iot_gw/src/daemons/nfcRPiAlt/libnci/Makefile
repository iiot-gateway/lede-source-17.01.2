# ------------------------------------------------------------------
# Nfc lib makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

NFC_CHIP ?= PN7150

ifeq ($(NFC_CHIP), PN7120)
libnci_chip_config_FLAGS += -DNFC_NXP_CHIP_TYPE=PN547C2 -DNXP_CHIP_NAME=\"pn7120\"
endif

ifeq ($(NFC_CHIP), PN7150)
libnci_chip_config_FLAGS += -DNFC_NXP_CHIP_TYPE=PN548C2 -DNXP_CHIP_NAME=\"pn7150\"
endif

EXTRA_CFLAGS = -DPN547C2=1 -DPN548C2=2 -DPN551C2=3 \
        -DNFC_NXP_NOT_OPEN_INCLUDED=TRUE \
        -DNXP_HW_SELF_TEST -DDEBUG \
        $(libnci_chip_config_FLAGS) \
        -DPH_NCI_NXP_HAL_ENABLE_FW_DOWNLOAD=FALSE \
        -DNXP_NFC_NATIVE_ENABLE_HCE=FALSE

INCLUDES = -Ilinux_libnfc-nci/src/include \
        -Ilinux_libnfc-nci/src/libnfc-nci/include \
        -Ilinux_libnfc-nci/src/libnfc-nci/gki/ulinux \
        -Ilinux_libnfc-nci/src/libnfc-nci/gki/common \
        -Ilinux_libnfc-nci/src/libnfc-nci/nfa/include \
        -Ilinux_libnfc-nci/src/libnfc-nci/nfa/int \
        -Ilinux_libnfc-nci/src/libnfc-nci/nfc/include \
        -Ilinux_libnfc-nci/src/libnfc-nci/nfc/int \
        -Ilinux_libnfc-nci/src/libnfc-nci/hal/include \
        -Ilinux_libnfc-nci/src/libnfc-nci/hal/int \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/utils \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/inc \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/common \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/dnld \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/hal \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/log \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/tml \
        -Ilinux_libnfc-nci/src/halimpl/pn54x/self-test \
        -Ilinux_libnfc-nci/src/service/interface \
        -Ilinux_libnfc-nci/src/service/utils \
        -Ilinux_libnfc-nci/src/service/extns/inc \
        -Ilinux_libnfc-nci/src/service/extns/src/common \
        -Ilinux_libnfc-nci/src/service/extns/src/mifare

OBJECTS = \
        linux_libnfc-nci/src/libnfc-nci/nfa/rw/nfa_rw_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/rw/nfa_rw_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/rw/nfa_rw_act.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/hci/nfa_hci_ci.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/hci/nfa_hci_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/hci/nfa_hci_act.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/hci/nfa_hci_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/hci/nfa_hci_utils.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/ce/nfa_ce_act.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/ce/nfa_ce_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/ce/nfa_ce_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/cho/nfa_cho_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/cho/nfa_cho_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/cho/nfa_cho_sm.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/cho/nfa_cho_util.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/p2p/nfa_p2p_act.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/p2p/nfa_p2p_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/p2p/nfa_p2p_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/snep/nfa_snep_act.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/snep/nfa_snep_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/snep/nfa_snep_default.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/snep/nfa_snep_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/sys/nfa_sys_cback.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/sys/nfa_sys_ptim.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/sys/nfa_sys_cfg.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/sys/nfa_sys_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/dm/nfa_dm_discover.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/dm/nfa_dm_act.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/dm/nfa_dm_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/dm/nfa_dm_ndef.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/dm/nfa_dm_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/dm/nfa_dm_cfg.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/ee/nfa_ee_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/ee/nfa_ee_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfa/ee/nfa_ee_act.o \
        linux_libnfc-nci/src/libnfc-nci/adaptation/libmain.o \
        linux_libnfc-nci/src/libnfc-nci/gki/common/gki_debug.o \
        linux_libnfc-nci/src/libnfc-nci/gki/common/gki_time.o \
        linux_libnfc-nci/src/libnfc-nci/gki/common/gki_buffer.o \
        linux_libnfc-nci/src/libnfc-nci/gki/ulinux/gki_ulinux.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/llcp/llcp_sdp.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/llcp/llcp_link.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/llcp/llcp_api.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/llcp/llcp_util.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/llcp/llcp_dlc.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/llcp/llcp_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_vs.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_task.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_ncif.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_test.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_ee.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nfc/nfc_utils.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nci/nci_hrcv.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/nci/nci_hmsgs.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_i93.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_t4t.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/tags_int.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/ce_t3t.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_t2t.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_t2t_ndef.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_t1t.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/ce_t4t.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_t3t.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/ce_main.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/tags/rw_t1t_ndef.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/ndef/ndef_utils.o \
        linux_libnfc-nci/src/libnfc-nci/nfc/ndef/ndef_cho_utils.o \
        linux_libnfc-nci/src/libnfc-nci/adaptation/NfcAdaptation.o \
        linux_libnfc-nci/src/libnfc-nci/adaptation/config.o \
        linux_libnfc-nci/src/libnfc-nci/adaptation/OverrideLog.o \
        linux_libnfc-nci/src/libnfc-nci/adaptation/android_logmsg.o \
        linux_libnfc-nci/src/libnfc-nci/adaptation/CrcChecksum.o \
        linux_libnfc-nci/src/halimpl/pn54x/tml/phDal4Nfc_messageQueueLib.o \
        linux_libnfc-nci/src/halimpl/pn54x/tml/phTmlNfc.o \
        linux_libnfc-nci/src/halimpl/pn54x/tml/phOsalNfc_Timer.o \
        linux_libnfc-nci/src/halimpl/pn54x/dnld/phNxpNciHal_Dnld.o \
        linux_libnfc-nci/src/halimpl/pn54x/dnld/phDnldNfc_Internal.o \
        linux_libnfc-nci/src/halimpl/pn54x/dnld/phDnldNfc.o \
        linux_libnfc-nci/src/halimpl/pn54x/dnld/phDnldNfc_Utils.o \
        linux_libnfc-nci/src/halimpl/pn54x/log/phNxpLog.o \
        linux_libnfc-nci/src/halimpl/pn54x/self-test/phNxpNciHal_SelfTest.o \
        linux_libnfc-nci/src/halimpl/pn54x/hal/phNxpNciHal_NfcDepSWPrio.o \
        linux_libnfc-nci/src/halimpl/pn54x/hal/phNxpNciHal_dta.o \
        linux_libnfc-nci/src/halimpl/pn54x/hal/phNxpNciHal_ext.o \
        linux_libnfc-nci/src/halimpl/pn54x/hal/phNxpNciHal_Kovio.o \
        linux_libnfc-nci/src/halimpl/pn54x/hal/phNxpNciHal.o \
        linux_libnfc-nci/src/halimpl/pn54x/utils/phNxpNciHal_utils.o \
        linux_libnfc-nci/src/service/utils/Mutex.o \
        linux_libnfc-nci/src/service/utils/IntervalTimer.o \
        linux_libnfc-nci/src/service/utils/CondVar.o \
        linux_libnfc-nci/src/service/interface/nativeNfcTag.o \
        linux_libnfc-nci/src/service/interface/NfcTag.o \
        linux_libnfc-nci/src/service/interface/nativeNdef.o \
        linux_libnfc-nci/src/service/interface/nativeNfcHandover.o \
        linux_libnfc-nci/src/service/interface/nativeNfcLlcp.o \
        linux_libnfc-nci/src/service/interface/nativeNfcManager.o \
        linux_libnfc-nci/src/service/interface/nativeNfcSnep.o \
        linux_libnfc-nci/src/service/interface/RoutingManager.o \
        linux_libnfc-nci/src/service/extns/src/mifare/phFriNfc_SmtCrdFmt.o \
        linux_libnfc-nci/src/service/extns/src/mifare/phNxpExtns_MifareStd.o \
        linux_libnfc-nci/src/service/extns/src/mifare/phFriNfc_MifStdFormat.o \
        linux_libnfc-nci/src/service/extns/src/mifare/phFriNfc_MifareStdMap.o \
        linux_libnfc-nci/src/service/extns/src/phNxpExtns.o \
        linux_libnfc-nci/src/service/linux_nfc_api.o \
        linux_libnfc-nci/src/service/linux_nfc_factory_api.o \
        linux_libnfc-nci/src/halimpl/pn54x/tml/phTmlNfc_i2c.o \
        linux_libnfc-nci/src/halimpl/pn54x/utils/phNxpConfig_rhm.o


# ------------------------------------------------------------------
        
        
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all : build
	
build: $(OBJECTS)
	mkdir -p lib
	ar -cvq lib/libnci.a $(OBJECTS)

clean:
	rm -f $(OBJECTS)
	rm -f lib/libnci.a

