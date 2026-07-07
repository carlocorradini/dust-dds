#include "Dummy.h"
#include "ddsc/dds.h"

#define MAX_SAMPLES 16

int main(int argc, char *argv[]) {
  const char *topic_name = "Dummy";

  const dds_entity_t participant = dds_create_participant(
      DDS_DOMAIN_DEFAULT, NULL /*qos*/, NULL /*listener*/);
  if (participant < 0) {
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
  }
  const dds_entity_t topic =
      dds_create_topic(participant, &interoperability_test_Dummy_desc,
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
  const size_t wsresultsize = 1U;
  rc = dds_waitset_wait(waitset, wsresults, wsresultsize, DDS_SECS(30));
  if (rc == 0) {
    DDS_FATAL("dds_waitset_wait: timeout: Subscription not matched");
  }
  if (rc != wsresultsize) {
    DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-rc));
  }

  while (true) {
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

    interoperability_test_Dummy *samples[MAX_SAMPLES] = {0};
    dds_sample_info_t infos[MAX_SAMPLES];

    rc = dds_take(data_reader, (void **)samples, infos, MAX_SAMPLES,
                  MAX_SAMPLES);
    if (rc < 0) {
      DDS_FATAL("dds_take: %s\n", dds_strretcode(-rc));
    }

    for (dds_return_t i = 0; i < rc; ++i) {
      const interoperability_test_Dummy *sample = samples[i];
      printf("take \"%s\": { id: \"%s\", value: %d }\n",
             interoperability_test_Dummy_desc.m_typename, sample->id,
             sample->value);
    }
  }
}
