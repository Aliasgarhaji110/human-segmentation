import numpy as np
from scipy import misc
import tensorflow as tf
import matplotlib.pyplot as plt
from segmentation_python.data_utils import DataSet
from segmentation_python.networks import SegmentationNetwork
import time
from segmentation_python import utils
from segmentation_python.initialize import _RESULT_PATH

def eval(dataset, batch_size, num_epochs, show_last_prediction = True, chkpt_interval = 1, override_tfrecords = None, load_from_chkpt=None):
    data_dim = dataset.data_dim
    depths, labels = dataset.get_batch_from_tfrecords_via_queue(batch_size=batch_size, num_epochs=num_epochs,
                                                                type='test', override_tfrecords = override_tfrecords)

    model = SegmentationNetwork(depths, data_dim)
    print('deconv_logits shape: ', model.net_class.deconv_logits.shape)
    predictions = model.get_predictions()
    print('prediction shape', predictions.shape)

    cross_entropy_loss = model.loss(labels)

    coord = tf.train.Coordinator()
    init_op = tf.group(tf.global_variables_initializer(),
                       tf.local_variables_initializer())

    timestamp = utils.get_timestamp()
    test_details_file_path = _RESULT_PATH + '%s' % timestamp + "_test_details.txt"
    utils.print_test_details(batch_size, num_epochs, override_tfrecords, load_from_chkpt,
                                 test_details_file_path)
    metrics_file_path = _RESULT_PATH + '%s' % timestamp + "_test_metrics.txt"
    image_result_part_path = _RESULT_PATH + '%s' % timestamp + "_test_image_"

    with tf.Session() as sess:
        sess.run(init_op)
        threads = tf.train.start_queue_runners(sess=sess, coord=coord)
        step = 0
        step_vector = []
        loss_vector = []
        acc_vector = []

        if load_from_chkpt:
            utils.load_checkpoint(sess, load_from_chkpt)
        else:
            print('Error! Pass a checkpoint to load')
            return

        start_time = time.time()
        try:
            #while not coord.should_stop():
            for i in range(10):
                # Run one step of the model.  The return values are
                # the activations from the `train_op` (which is
                # discarded) and the `loss` op.  To inspect the values
                # of your ops or variables, you may include them in
                # the list passed to sess.run() and the value tensors
                # will be returned in the tuple from the call.
                loss, pred, corr_depth, corr_label = sess.run(
                    [cross_entropy_loss, predictions, depths, labels])

                loss_value = np.mean(loss)

                # pred = sess.run(model.get_predictions())
                # last_label = sess.run(labels)

                # print('pred shape: ', pred.shape)
                # print('last label shape: ', last_label.shape)


                # Print an overview fairly often.
                if step % chkpt_interval == 0:
                    acc = utils.accuracy_per_pixel(pred, corr_label)
                    utils.print_metrics(loss=loss_value,accuracy=acc,step=step,metrics_file_path=metrics_file_path)
                    step_vector.append(step)
                    loss_vector.append(loss_value)
                    utils.visualize_predictions(pred[0],np.squeeze(corr_label[0]),np.squeeze(corr_depth[0]),path = image_result_part_path + str(step) + '.png')


                step += 1

        except tf.errors.OutOfRangeError:
            print('Done training for %d epochs, %d steps.' % (1, step))
        finally:
            # When done, ask the threads to stop.
            acc = utils.accuracy_IOU(pred, corr_label)
            utils.print_metrics(loss=loss_value,accuracy=acc,step=step,metrics_file_path=metrics_file_path)
            step_vector.append(step)
            loss_vector.append(loss_value)

            coord.request_stop()

            end_time = time.time()
            print('Time taken for training: ', end_time - start_time)

        # Wait for threads to finish.
        coord.join(threads)

        if show_last_prediction:
            utils.visualize_predictions(pred[0],np.squeeze(corr_label[0]),np.squeeze(corr_depth[0]),path = image_result_part_path + str(step) + '.png')

        utils.plot_loss(step_vector, loss_vector, timestamp, 'test')

if __name__ == '__main__':
    dataset = DataSet(num_poses=53, num_angles=360, max_records_in_tfrec_file=3600, val_fraction=0.01,
                      test_fraction=0.01)
    batch_size = 1
    num_epochs = 1
    override_tfrecords = ['/home/neha/Documents/TUM_Books/projects/IDP/segmentation/segmentation_python/data/TfRecordFile_test_0.tfrecords']
    chkpt = '/home/neha/Documents/TUM_Books/projects/IDP/segmentation/segmentation_python/chkpt/2017_08_28_13_42_checkpoint3600.ckpt'

    eval(dataset=dataset,batch_size=batch_size,num_epochs=num_epochs, override_tfrecords=override_tfrecords, load_from_chkpt = chkpt)
