#include "Dummy.h"
#include "ddsc/dds.h"
#include <assert.h>
#include <float.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MAX_SAMPLES 1

int main() {
  const char *topic_name = "Dummy";

  const dds_entity_t participant = dds_create_participant(
      DDS_DOMAIN_DEFAULT, NULL /*qos*/, NULL /*listener*/);
  if (participant < 0) {
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
  }
  const dds_entity_t topic =
      dds_create_topic(participant, &interoperability_test_MyStruct_desc,
                       topic_name, NULL /*qos*/, NULL /*listener*/);
  if (topic < 0) {
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
  }
  dds_qos_t *qos = dds_create_qos();
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
  dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);

  const dds_entity_t data_reader =
      dds_create_reader(participant, topic, qos, NULL /*listener*/);
  if (data_reader < 0) {
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-data_reader));
  }

  dds_return_t rc;

  rc = dds_set_status_mask(data_reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
  if (rc != DDS_RETCODE_OK) {
    DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));
  }

  dds_entity_t waitset = dds_create_waitset(participant);

  rc = dds_waitset_attach(waitset, data_reader, data_reader);
  if (rc != DDS_RETCODE_OK) {
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-rc));
  }

  dds_attach_t wsresults[1];
  const dds_return_t wsresultsize = 1U;
  rc = dds_waitset_wait(waitset, wsresults, wsresultsize, DDS_SECS(3660));
  if (rc == 0) {
    DDS_FATAL("dds_waitset_wait: timeout: Subscription not matched");
  }
  if (rc != wsresultsize) {
    DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-rc));
  }

  rc = dds_set_status_mask(data_reader, DDS_DATA_AVAILABLE_STATUS);
  if (rc != DDS_RETCODE_OK) {
    DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));
  }
  rc = dds_waitset_wait(waitset, wsresults, wsresultsize, DDS_SECS(30));
  if (rc == 0) {
    DDS_FATAL("dds_waitset_wait: timeout: No data received");
  }
  if (rc != wsresultsize) {
    DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-rc));
  }

  interoperability_test_MyStruct *msg;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  samples[0] = interoperability_test_MyStruct__alloc();

  rc = dds_read(data_reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
  if (rc < 0) {
    DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
  }

  if ((rc > 0) && (infos[0].valid_data)) {
    msg = (interoperability_test_MyStruct *)samples[0];
    printf("Received: \"%s\" { ",
           interoperability_test_MyStruct_desc.m_typename);

    printf("id: %lu, ", msg->id);
    printf("unions: [");
    for (uint32_t i = 0; i < msg->unions._length; ++i) {
      assert(msg->unions._buffer[i]._d == 0);

      printf("element { ");
      printf("values: [");
      for (uint32_t j = 0; j < msg->unions._buffer[i]._u.element.values._length;
           ++j) {
        printf("%lu", msg->unions._buffer[i]._u.element.values._buffer[j]);
        if (j != (msg->unions._buffer[i]._u.element.values._length - 1)) {
          printf(", ");
        }
      }
      printf("], ");
      printf("maybe: ");
      if (msg->unions._buffer[i]._u.element.maybe != NULL) {
        printf("Some(%lu)", *msg->unions._buffer[i]._u.element.maybe);
      } else {
        printf("None");
      }
      printf(" }");

      if (i != (msg->unions._length - 1)) {
        printf(", ");
      }
    }
    printf("]}\n");
  }

  // Sleep to allow sending acknowledgements
  dds_sleepfor(DDS_SECS(2));
}
