/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#include <stdint.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "device.h"
#include "awf.h"

// Atomic Write File

// encode service
int awf_encode_apdu(
  uint8_t *apdu, 
  uint8_t invoke_id,
  BACNET_ATOMIC_WRITE_FILE_DATA *data)
{
  int apdu_len = 0; // total length of the apdu, return value

  if (apdu)
  {
    apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
    apdu[1] = encode_max_segs_max_apdu(0, Device_Max_APDU_Length_Accepted());
    apdu[2] = invoke_id; 
    apdu[3] = SERVICE_CONFIRMED_ATOMIC_WRITE_FILE;     // service choice
    apdu_len = 4;
    apdu_len += encode_context_object_id(&apdu[apdu_len], 0,
      data->object_type, data->object_instance);
    apdu_len += encode_context_enumerated(&apdu[apdu_len], 1,
      data->access);
    apdu_len += encode_opening_tag(&apdu[apdu_len], 2);
    switch (data->access)
    {
      case FILE_RECORD_ACCESS:
        apdu_len += encode_tagged_signed(&apdu[apdu_len],
          data->type.record.fileStartRecord);
        apdu_len += encode_tagged_unsigned(&apdu[apdu_len],
          data->type.record.returnedRecordCount);
        break;
      case FILE_STREAM_ACCESS:
        apdu_len += encode_tagged_signed(&apdu[apdu_len],
          data->type.stream.fileStartPosition);
        break;
      default:
        break;
    }
    apdu_len += encode_tagged_octet_string(&apdu[apdu_len],
      data->fileData, data->fileDataLength);
    apdu_len += encode_closing_tag(&apdu[apdu_len], 2);
  }
  
  return apdu_len;
}

// decode the service request only
int awf_decode_service_request(
  uint8_t *apdu,
  unsigned apdu_len,
  BACNET_ATOMIC_WRITE_FILE_DATA *data)
{
  int len = 0;
  int tag_len = 0;
  uint8_t tag_number = 0;
  uint32_t len_value_type = 0;
  int type = 0; // for decoding
  int access = 0; // for decoding

  // check for value pointers
  if (apdu_len && data)
  {
    // Tag 0: Object ID         
    if (!decode_is_context_tag(&apdu[len++], 0))
      return -1;
    len += decode_object_id(&apdu[len], &type, &data->object_instance);
    data->object_type = type;
    // Tag 1: Access
    len += decode_tag_number_and_value(&apdu[len],
        &tag_number, &len_value_type);
    if (tag_number != 1)
      return -1;
    len += decode_enumerated(&apdu[len], len_value_type, &access);
    data->access = access;
    // Tag 2: Opening Context Tag
    if (!decode_is_opening_tag_number(&apdu[len], 2))
      return -1;
    // a tag number of 2 is not extended so only one octet
    len++;
    if (access == FILE_RECORD_ACCESS)
    {
      // fileStartRecord
      tag_len = decode_tag_number_and_value(&apdu[len],
        &tag_number, &len_value_type);
      len += tag_len;
      if (tag_number != BACNET_APPLICATION_TAG_SIGNED_INT)
        return -1;
      len += decode_signed(&apdu[len],
        len_value_type,
        &data->type.record.fileStartRecord);
      // returnedRecordCount
      tag_len = decode_tag_number_and_value(&apdu[len],
        &tag_number, &len_value_type);
      len += tag_len;
      if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT)
        return -1;
      len += decode_unsigned(&apdu[len],
        len_value_type,
        &data->type.record.returnedRecordCount);
      // fileData
      tag_len = decode_tag_number_and_value(&apdu[len],
        &tag_number, &len_value_type);
      len += tag_len;
      if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING)
        return -1;
      len += decode_octet_string(&apdu[len],
        len_value_type,
        data->fileData,
        data->fileDataLength);
      data->fileDataLength = len_value_type;
    }
    else if (access == FILE_STREAM_ACCESS)
    {
      // fileStartPosition
      tag_len = decode_tag_number_and_value(&apdu[len],
        &tag_number, &len_value_type);
      len += tag_len;
      if (tag_number != BACNET_APPLICATION_TAG_SIGNED_INT)
        return -1;
      len += decode_signed(&apdu[len],
        len_value_type,
        &data->type.stream.fileStartPosition);
      // fileData
      tag_len = decode_tag_number_and_value(&apdu[len],
        &tag_number, &len_value_type);
      len += tag_len;
      if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING)
        return -1;
      len += decode_octet_string(&apdu[len],
        len_value_type,
        data->fileData,
        data->fileDataLength);
      data->fileDataLength = len_value_type;
    }
    else
      return -1;
    if (!decode_is_closing_tag_number(&apdu[len], 2))
      return -1;
    // a tag number of 2 is not extended so only one octet
    len++;
  }

  return len;
}

int awf_decode_apdu(
  uint8_t *apdu,
  unsigned apdu_len,
  uint8_t *invoke_id,
  BACNET_ATOMIC_WRITE_FILE_DATA *data)
{
  int len = 0;
  unsigned offset = 0;

  if (!apdu)
    return -1;
  // optional checking - most likely was already done prior to this call
  if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
    return -1;
  //  apdu[1] = encode_max_segs_max_apdu(0, Device_Max_APDU_Length_Accepted());
  *invoke_id = apdu[2]; /* invoke id - filled in by net layer */
  if (apdu[3] != SERVICE_CONFIRMED_ATOMIC_WRITE_FILE)
    return -1;
  offset = 4;

  if (apdu_len > offset)
  {
    len = awf_decode_service_request(
      &apdu[offset],
      apdu_len - offset,
      data);
  }
  
  return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testAtomicWriteFileAccess(Test * pTest,
  BACNET_ATOMIC_WRITE_FILE_DATA *data)
{
  BACNET_ATOMIC_WRITE_FILE_DATA test_data = {0};
  uint8_t apdu[480] = {0};
  int len = 0;
  int apdu_len = 0;
  uint8_t invoke_id = 128;
  uint8_t test_invoke_id = 0;
  uint8_t test_octet_string[128] = "bingo!";

  len = awf_encode_apdu(
    &apdu[0],
    invoke_id,
    data);
  ct_test(pTest, len != 0);
  apdu_len = len;

  test_data.fileData = test_octet_string;
  test_data.fileDataLength = sizeof(test_octet_string);
  len = awf_decode_apdu(
    &apdu[0],
    apdu_len,
    &test_invoke_id,
    &test_data);
  ct_test(pTest, len != -1);
  ct_test(pTest, test_data.object_type == data->object_type);
  ct_test(pTest, test_data.object_instance == data->object_instance);
  ct_test(pTest, test_data.access == data->access);
  if (test_data.access == FILE_STREAM_ACCESS)
  {
    ct_test(pTest, test_data.type.stream.fileStartPosition ==
      data->type.stream.fileStartPosition);
  }
  else if (test_data.access == FILE_RECORD_ACCESS)
  {
    ct_test(pTest, test_data.type.record.fileStartRecord ==
      data->type.record.fileStartRecord);
    ct_test(pTest, test_data.type.record.returnedRecordCount ==
      data->type.record.returnedRecordCount);
  }
  ct_test(pTest, test_data.fileDataLength ==
    data->fileDataLength);
  ct_test(pTest, memcmp(
    &test_data.fileData[0],
    &data->fileData[0],
    test_data.fileDataLength) == 0);
}

void testAtomicWriteFile(Test * pTest)
{
  BACNET_ATOMIC_WRITE_FILE_DATA data = {0};
  uint8_t *test_octet_string = "Joshua-Mary-Anna-Christopher";
  
  data.object_type = OBJECT_FILE;
  data.object_instance = 1;
  data.access = FILE_STREAM_ACCESS;
  data.type.stream.fileStartPosition = 0;
  data.fileData = test_octet_string;
  data.fileDataLength = strlen(test_octet_string);
  testAtomicWriteFileAccess(pTest, &data);

  data.object_type = OBJECT_FILE;
  data.object_instance = 1;
  data.access = FILE_RECORD_ACCESS;
  data.type.record.fileStartRecord = 1;
  data.type.record.returnedRecordCount = 2;
  data.fileData = test_octet_string;
  data.fileDataLength = strlen(test_octet_string);
  testAtomicWriteFileAccess(pTest, &data);
  
  return;
}

#ifdef TEST_ATOMIC_WRITE_FILE
uint16_t Device_Max_APDU_Length_Accepted(void)
{
  return MAX_APDU;
}

int main(void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet AtomicWriteFile", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAtomicWriteFile);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif                          /* TEST_WRITE_PROPERTY */
#endif                          /* TEST */
