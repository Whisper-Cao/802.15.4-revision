#ifndef INCLUDED_GR_IEEE802_15_4_MESSAGE_GENERATOR_H
#define INCLUDED_GR_IEEE802_15_4_MESSAGE_GENERATOR_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>

namespace gr {
namespace ieee802_15_4 {

    class IEEE802_15_4_API message_generator : virtual public block
    {
    public:
      // gr::blocks::message_strobe::sptr
      typedef boost::shared_ptr<message_generator> sptr;

      /*!
       * Make a message stobe block to send message \p msg every \p
       * period_ms milliseconds.
       *
       * \param msg The message to send as a PMT.
       * \param period_ms the time period in milliseconds in which to
       *                  send \p msg.
       */
      static sptr make();

    };
}
}
#endif
