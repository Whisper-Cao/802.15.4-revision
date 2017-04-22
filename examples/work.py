#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: IEEE 802.15.4 Transceiver using OQPSK PHY
# Generated: Thu Mar  9 13:35:58 2017
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from PyQt4 import Qt
from PyQt4.QtCore import QObject, pyqtSlot
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import qtgui
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from ieee802_15_4_oqpsk_phy import ieee802_15_4_oqpsk_phy  # grc-generated hier_block
from optparse import OptionParser
import foo
import ieee802_15_4
import pmt
import sip
import time

#case = 0
#isFirst = 1
#flag_1 = 0
#flag_2 = 0

class transceiver_OQPSK(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "IEEE 802.15.4 Transceiver using OQPSK PHY")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("IEEE 802.15.4 Transceiver using OQPSK PHY")
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "transceiver_OQPSK")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.tx_gain = tx_gain = 0.75
        self.rx_gain = rx_gain = 0.75
        self.freq = freq = 2480000000

        ##################################################
        # Blocks
        ##################################################
        self._tx_gain_range = Range(0, 1, 0.01, 0.75, 200)
        self._tx_gain_win = RangeWidget(self._tx_gain_range, self.set_tx_gain, "tx_gain", "counter_slider", float)
        self.top_layout.addWidget(self._tx_gain_win)
        self._rx_gain_range = Range(0, 1, 0.01, 0.75, 200)
        self._rx_gain_win = RangeWidget(self._rx_gain_range, self.set_rx_gain, "rx_gain", "counter_slider", float)
        self.top_layout.addWidget(self._rx_gain_win)
        self._freq_options = [1000000 * (2400 + 5 * (i - 10)) for i in range(11, 27)]
        self._freq_labels = [str(i) for i in range(11, 27)]
        self._freq_tool_bar = Qt.QToolBar(self)
        self._freq_tool_bar.addWidget(Qt.QLabel("Channel"+": "))
        self._freq_combo_box = Qt.QComboBox()
        self._freq_tool_bar.addWidget(self._freq_combo_box)
        for label in self._freq_labels: self._freq_combo_box.addItem(label)
        self._freq_callback = lambda i: Qt.QMetaObject.invokeMethod(self._freq_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._freq_options.index(i)))
        self._freq_callback(self.freq)
        self._freq_combo_box.currentIndexChanged.connect(
        	lambda i: self.set_freq(self._freq_options[i]))
        self.top_layout.addWidget(self._freq_tool_bar)
        self.uhd_usrp_source_1 = uhd.usrp_source(
        	",".join(("addr=192.168.10.2", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_1.set_samp_rate(400000)
        self.uhd_usrp_source_1.set_center_freq(freq, 0)
        self.uhd_usrp_source_1.set_normalized_gain(rx_gain, 0)
        self.uhd_usrp_source_1.set_antenna("J2", 0)
        self.uhd_usrp_sink_1 = uhd.usrp_sink(
        	",".join(("addr=192.168.10.3", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_1.set_samp_rate(400000)
        self.uhd_usrp_sink_1.set_center_freq(freq, 0)
        self.uhd_usrp_sink_1.set_gain(tx_gain, 0)
        self.uhd_usrp_sink_1.set_antenna("J2", 0)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
        	1024, #size
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	0, #fc
        	4e6, #bw
        	"", #name
        	1 #number of inputs
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis(-140, 10)
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(False)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)
        
        if not True:
          self.qtgui_freq_sink_x_0.disable_legend()
        
        if "complex" == "float" or "complex" == "msg_float":
          self.qtgui_freq_sink_x_0.set_plot_pos_half(not True)
        
        labels = ["", "", "", "", "",
                  "", "", "", "", ""]
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
                  "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])
        
        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_freq_sink_x_0_win)
        self.ieee802_15_4_rime_stack_0 = ieee802_15_4.rime_stack(([129]), ([131]), ([132]), ([23,42]))
        self.ieee802_15_4_oqpsk_phy_0 = ieee802_15_4_oqpsk_phy()
        self.ieee802_15_4_mac_0 = ieee802_15_4.mac(True,True)
        self.foo_wireshark_connector_1_0_0 = foo.wireshark_connector(195, False)
        self.foo_wireshark_connector_1_0 = foo.wireshark_connector(195, False)
        self.foo_wireshark_connector_1 = foo.wireshark_connector(195, False)
        #self.blocks_message_strobe_0 = blocks.message_strobe(pmt.intern("I"), 1000)
        self.blocks_file_sink_1_0_0 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensorPDURec.pcap", False)
        self.blocks_file_sink_1_0_0.set_unbuffered(True)
        self.blocks_file_sink_1_0 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensorAppRec.pcap", False)
        self.blocks_file_sink_1_0.set_unbuffered(True)
        self.blocks_file_sink_1 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensor.pcap", False)
        self.blocks_file_sink_1.set_unbuffered(True)
        self.ieee802_15_4_message_generator_0 = ieee802_15_4.message_generator()
        ##################################################
        # Connections
        ##################################################
        #self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.ieee802_15_4_rime_stack_0, 'bcin'))    
        self.msg_connect((self.ieee802_15_4_message_generator_0, 'strobe'), (self.ieee802_15_4_rime_stack_0, 'bcin'))    
        self.msg_connect((self.ieee802_15_4_mac_0, 'pdu out'), (self.foo_wireshark_connector_1, 'in'))    
        self.msg_connect((self.ieee802_15_4_mac_0, 'app out'), (self.foo_wireshark_connector_1_0, 'in'))       
        self.msg_connect((self.ieee802_15_4_mac_0, 'pdu out'), (self.ieee802_15_4_oqpsk_phy_0, 'txin'))    
        self.msg_connect((self.ieee802_15_4_mac_0, 'app out'), (self.ieee802_15_4_rime_stack_0, 'fromMAC'))    
        self.msg_connect((self.ieee802_15_4_oqpsk_phy_0, 'rxout'), (self.foo_wireshark_connector_1_0_0, 'in'))    
        self.msg_connect((self.ieee802_15_4_oqpsk_phy_0, 'rxout'), (self.ieee802_15_4_mac_0, 'pdu in'))    
        self.msg_connect((self.ieee802_15_4_rime_stack_0, 'toMAC'), (self.ieee802_15_4_mac_0, 'app in'))    
        self.connect((self.foo_wireshark_connector_1, 0), (self.blocks_file_sink_1, 0))    
        self.connect((self.foo_wireshark_connector_1_0, 0), (self.blocks_file_sink_1_0, 0))    
        self.connect((self.foo_wireshark_connector_1_0_0, 0), (self.blocks_file_sink_1_0_0, 0))    
        self.connect((self.ieee802_15_4_oqpsk_phy_0, 0), (self.qtgui_freq_sink_x_0, 0))    
        self.connect((self.ieee802_15_4_oqpsk_phy_0, 0), (self.uhd_usrp_sink_1, 0))    
        self.connect((self.uhd_usrp_source_1, 0), (self.ieee802_15_4_oqpsk_phy_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "transceiver_OQPSK")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()


    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain
        self.uhd_usrp_sink_1.set_gain(self.tx_gain, 0)
        	

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain
        self.uhd_usrp_source_1.set_normalized_gain(self.rx_gain, 0)
        	

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self._freq_callback(self.freq)
        self.uhd_usrp_source_1.set_center_freq(self.freq, 0)
        self.uhd_usrp_sink_1.set_center_freq(self.freq, 0)
	
    def starts(self):
        global ct
        ct = ct + 1
        self.start()
        #self.start()


class transceiver_OQPSK_th2(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "IEEE 802.15.4 Feedback using OQPSK PHY")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("IEEE 802.15.4 Feedback using OQPSK PHY")
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "transceiver_OQPSK")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.tx_gain = tx_gain = 0.75
        self.rx_gain = rx_gain = 0.75
        self.freq = freq = 2480000000

        ##################################################
        # Blocks
        ##################################################
        self._tx_gain_range = Range(0, 1, 0.01, 0.75, 200)
        self._tx_gain_win = RangeWidget(self._tx_gain_range, self.set_tx_gain, "tx_gain", "counter_slider", float)
        self.top_layout.addWidget(self._tx_gain_win)
        self._rx_gain_range = Range(0, 1, 0.01, 0.75, 200)
        self._rx_gain_win = RangeWidget(self._rx_gain_range, self.set_rx_gain, "rx_gain", "counter_slider", float)
        self.top_layout.addWidget(self._rx_gain_win)
        self._freq_options = [1000000 * (2400 + 5 * (i - 10)) for i in range(11, 27)]
        self._freq_labels = [str(i) for i in range(11, 27)]
        self._freq_tool_bar = Qt.QToolBar(self)
        self._freq_tool_bar.addWidget(Qt.QLabel("Channel"+": "))
        self._freq_combo_box = Qt.QComboBox()
        self._freq_tool_bar.addWidget(self._freq_combo_box)
        for label in self._freq_labels: self._freq_combo_box.addItem(label)
        self._freq_callback = lambda i: Qt.QMetaObject.invokeMethod(self._freq_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._freq_options.index(i)))
        self._freq_callback(self.freq)
        self._freq_combo_box.currentIndexChanged.connect(
        	lambda i: self.set_freq(self._freq_options[i]))
        self.top_layout.addWidget(self._freq_tool_bar)
        self.uhd_usrp_source_1 = uhd.usrp_source(
        	",".join(("addr=192.168.10.3", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_source_1.set_samp_rate(400000)
        self.uhd_usrp_source_1.set_center_freq(freq, 0)
        self.uhd_usrp_source_1.set_normalized_gain(rx_gain, 0)
        self.uhd_usrp_source_1.set_antenna("J2", 0)
        self.uhd_usrp_sink_1 = uhd.usrp_sink(
        	",".join(("addr=192.168.10.2", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        )
        self.uhd_usrp_sink_1.set_samp_rate(400000)
        self.uhd_usrp_sink_1.set_center_freq(freq, 0)
        self.uhd_usrp_sink_1.set_gain(tx_gain, 0)
        self.uhd_usrp_sink_1.set_antenna("J2", 0)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
        	1024, #size
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	0, #fc
        	4e6, #bw
        	"", #name
        	1 #number of inputs
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis(-140, 10)
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(False)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)
        
        if not True:
          self.qtgui_freq_sink_x_0.disable_legend()
        
        if "complex" == "float" or "complex" == "msg_float":
          self.qtgui_freq_sink_x_0.set_plot_pos_half(not True)
        
        labels = ["", "", "", "", "",
                  "", "", "", "", ""]
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
                  "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])
        
        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_freq_sink_x_0_win)
        self.ieee802_15_4_rime_stack_0 = ieee802_15_4.rime_stack(([129]), ([131]), ([132]), ([23,42]))
        self.ieee802_15_4_oqpsk_phy_0 = ieee802_15_4_oqpsk_phy()
        self.ieee802_15_4_feedback_generator_0 = ieee802_15_4.feedback_generator()
        self.ieee802_15_4_mac_0 = ieee802_15_4.mac(True,False)
        self.foo_wireshark_connector_1_0_0 = foo.wireshark_connector(195, False)
        self.foo_wireshark_connector_1_0 = foo.wireshark_connector(195, False)
        self.foo_wireshark_connector_1 = foo.wireshark_connector(195, False)
        
        #
        #self.blocks_message_strobe_0 = blocks.message_strobe(pmt.intern("A"), 1000)
        #
        '''
        self.blocks_file_sink_1_0_0 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensorPDURec.pcap", False)
        self.blocks_file_sink_1_0_0.set_unbuffered(True)
        self.blocks_file_sink_1_0 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensorAppRec.pcap", False)
        self.blocks_file_sink_1_0.set_unbuffered(True)
        self.blocks_file_sink_1 = blocks.file_sink(gr.sizeof_char*1, "/tmp/sensor.pcap", False)
        self.blocks_file_sink_1.set_unbuffered(True)
'''
        ##################################################
        # Connections
        ##################################################
  	self.msg_connect((self.ieee802_15_4_feedback_generator_0, 'out to RS'), (self.ieee802_15_4_rime_stack_0, 'bcin'))          
	#self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.ieee802_15_4_rime_stack_0, 'bcin'))    
        #self.msg_connect((self.ieee802_15_4_mac_0, 'pdu out'), (self.foo_wireshark_connector_1, 'in'))    
        #self.msg_connect((self.ieee802_15_4_mac_0, 'app out'), (self.foo_wireshark_connector_1_0, 'in'))    
        #self.msg_connect((self.ieee802_15_4_mac_0, 'app out'), (self.ieee802_15_4_message_feedback_0, 'from pdu in'))    
        self.msg_connect((self.ieee802_15_4_mac_0, 'pdu out'), (self.ieee802_15_4_oqpsk_phy_0, 'txin'))    
        self.msg_connect((self.ieee802_15_4_mac_0, 'app out'), (self.ieee802_15_4_rime_stack_0, 'fromMAC'))    
        #self.msg_connect((self.ieee802_15_4_oqpsk_phy_0, 'rxout'), (self.foo_wireshark_connector_1_0_0, 'in'))    
        self.msg_connect((self.ieee802_15_4_oqpsk_phy_0, 'rxout'), (self.ieee802_15_4_mac_0, 'pdu in'))    
        self.msg_connect((self.ieee802_15_4_rime_stack_0, 'toMAC'), (self.ieee802_15_4_mac_0, 'app in'))    
       # self.connect((self.foo_wireshark_connector_1, 0), (self.blocks_file_sink_1, 0))    
       # self.connect((self.foo_wireshark_connector_1_0, 0), (self.blocks_file_sink_1_0, 0))    
       # self.connect((self.foo_wireshark_connector_1_0_0, 0), (self.blocks_file_sink_1_0_0, 0))    
        self.connect((self.ieee802_15_4_oqpsk_phy_0, 0), (self.qtgui_freq_sink_x_0, 0))    
        self.connect((self.ieee802_15_4_oqpsk_phy_0, 0), (self.uhd_usrp_sink_1, 0))    
        self.connect((self.uhd_usrp_source_1, 0), (self.ieee802_15_4_oqpsk_phy_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "transceiver_OQPSK")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()


    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain
        self.uhd_usrp_sink_1.set_gain(self.tx_gain, 0)
        	

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain
        self.uhd_usrp_source_1.set_normalized_gain(self.rx_gain, 0)
        	

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self._freq_callback(self.freq)
        self.uhd_usrp_source_1.set_center_freq(self.freq, 0)
        self.uhd_usrp_sink_1.set_center_freq(self.freq, 0)
	
    def starts(self):
        global ct
        ct = ct + 1
        self.start()
        #self.start()


def main(top_block_cls=transceiver_OQPSK, options=None):
    isFirst = 1
    isSecond = 0
    flag_1 = 0
    seq=0
    seq2=0
    prev_seq=0
    prev_seq2=0
   
    pac_num = 0


    top_clock_ack = transceiver_OQPSK_th2;

    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print "Error: failed to enable real-time scheduling."

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    print "working!!!!!!!!"

    f = open('/home/captain/test/seq/seq_1','w+')
    f.write("0")
    f.close()
    f = open('/home/captain/test/seq/seq_2','w+')
    f.write("0")
    f.close()
    f2 = open('/home/captain/test/acknum','w+')
    f2.write("0")
    f2.close()
    f2 = open('/home/captain/test/transceiver/seq_num',"w+")
    f2.write("0")
    f2.close()
    f3 = open('/home/captain/test/transceiver/send_num',"w")
    tb = top_block_cls()
    tb2 = top_clock_ack()
    road = 0;
    t=2000;
    k = 2000;
#seq represents the number of received packets by 2 from 1
#seq2 represents the # of received packets by 1 from 2
    while True:
        if isFirst == 1:
            print "===============first in==================="
            pac_num = pac_num + 1
            isFirst = 0
            t = k
            print "pac num is:" + str(pac_num)
            tb.start()

        if prev_seq == seq - 1 and isFirst == 0:
            print "===============send packet================"
            pac_num = pac_num + 1
            prev_seq = seq
            print "pac num is:" + str(pac_num)
            t = k
            tb.lock()
            tb.unlock()

        if t == 0 and isFirst == 0:
            print "==============retransmit=================="
            pac_num = pac_num + 1
            prev_seq = seq
            print "pac num is:" + str(pac_num)
            t = k
            tb.lock()
            tb.unlock()
        t = t - 1
        time.sleep(0.005)
        f = open('/home/captain/test/seq/seq_1','r')
        str2 = f.read()
        if str2:
            seq = int(str2)
    
    def quitting():
        tb.stop()
        tb.wait()
      #  print "pac_num: " + str(pac_num)
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()
        

'''         
        if isFirst == 1:
            print "==============first in===================="
            road = 0
            isFirst = 0
            isSecond = 1
            t = k
            pac_num = pac_num + 1
            print "pac num is:" + str(pac_num)
            tb.start()
        if prev_seq == seq-1 and isSecond == 1 and isFirst == 0:
            print "==============Second in==================="
            road = 1
            isSecond = 0
            flag_1 = 0
            prev_seq = seq
            t = k
            
          
            tb.lock()#lock means it stop until we unlock it
            tb2.start()
        if prev_seq2 == seq2-1 and isFirst == 0 and isSecond == 0:
            print "==============data packet working1============"
            road = 2
            tb2.lock()
            flag_1 = 1
            prev_seq2 = seq2
            t = k
            pac_num = pac_num + 1
            print "pac num is:" + str(pac_num)    
            tb.unlock()
        if prev_seq == seq-1 and isFirst == 0 and isSecond == 0:
            print "=============ack working=================="
            road = 3
            tb.lock()
            t = k
            prev_seq = seq
            flag_1 = 0 
            tb2.unlock()
        time.sleep(0.0002)
        f = open('/home/captain/test/seq/seq_1','r')
        f2 = open('/home/captain/test/seq/seq_2','r')
        str1 = f.read()
        str2 = f2.read()
        if str1:
            seq = int(str1)
        if str2:
            seq2 = int(str2)
       # print "================="
            #print "("+str(case2)+','+str(case) + ')'
       # print "================="
        f.close()
        f2.close()
        if t == 0:
            print str(seq)+","+str(seq2)
            if road == 0 or road == 2:#stuck in state 0 or 2
                print "=============data packet working2==========================================================="
                pac_num = pac_num+1
                print "pac num is:" + str(pac_num)
                
             
                tb.lock()
                road = 2
                t = k
                prev_seq = seq
                prev_seq2 = seq2
                seq2 = 0
                tb.unlock()
            if road == 1 or road == 3:#stuck in state 1 or 3
                print "=============data packet working3============================================================"
                pac_num = pac_num + 1
                print "pac num is:" + str(pac_num)
                tb2.lock()
                road = 2
                t = k
                prev_seq = seq
                prev_seq2 = seq2
                tb.unlock()
            isFirst = 0
            isSecond = 0
        t = t - 1

   # tb.starts()
   # tb.show()


    def quitting():
        tb.stop()
        tb.wait()
      #  print "pac_num: " + str(pac_num)
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()
'''

if __name__ == '__main__':
    main()
