// Release portability/documentation changes: 2026-10-17; firmware behavior and assets unchanged.
// SPDX-License-Identifier: GPL-3.0-only
// Authorized USB-device recording only; never selects a microphone fallback.
import AVFoundation
import Foundation
let args = CommandLine.arguments
if args.count != 4 { fatalError("Usage: fm1-record-audio output.mov seconds stop-file") }
let path = args[1], seconds = Double(args[2])!
if AVCaptureDevice.authorizationStatus(for: .audio) == .notDetermined {
 let permission=DispatchSemaphore(value:0)
 AVCaptureDevice.requestAccess(for:.audio){_ in permission.signal()};permission.wait()
}
guard AVCaptureDevice.authorizationStatus(for: .audio) == .authorized else { fatalError("Allow audio access for the launching app in macOS Privacy & Security, then retry. Only FoMni FM-1 is selected; there is no microphone fallback.") }
guard let device = AVCaptureDevice.devices(for: .audio).first(where: {$0.localizedName == "FoMni FM-1"}) else { fatalError("FoMni FM-1 USB audio not connected; no fallback input selected") }
let writer = try AVAssetWriter(outputURL: URL(fileURLWithPath:path), fileType: .mov)
let sink = AVAssetWriterInput(mediaType:.audio, outputSettings:[AVFormatIDKey:kAudioFormatLinearPCM,AVSampleRateKey:44100,AVNumberOfChannelsKey:2,AVLinearPCMBitDepthKey:16,AVLinearPCMIsFloatKey:false,AVLinearPCMIsBigEndianKey:false,AVLinearPCMIsNonInterleaved:false])
sink.expectsMediaDataInRealTime = true
writer.add(sink)
final class Recorder: NSObject, AVCaptureAudioDataOutputSampleBufferDelegate {
 let writer: AVAssetWriter, sink: AVAssetWriterInput
 var first: Double?, count = 0, dropped = 0, rows = [[String:Double]]()
 init(_ w:AVAssetWriter,_ s:AVAssetWriterInput){writer=w;sink=s}
 func captureOutput(_ output:AVCaptureOutput,didOutput buffer:CMSampleBuffer,from connection:AVCaptureConnection){
  let pts=CMSampleBufferGetPresentationTimeStamp(buffer)
  if first == nil {first=pts.seconds;writer.startSession(atSourceTime:pts)}
  let ok=sink.isReadyForMoreMediaData && sink.append(buffer)
  if !ok {dropped += 1}
  count += CMSampleBufferGetNumSamples(buffer)
  rows.append(["pts_seconds":pts.seconds,"received_host_seconds":CMClockGetTime(CMClockGetHostTimeClock()).seconds,"samples":Double(CMSampleBufferGetNumSamples(buffer)),"written":ok ? 1:0])
 }
}
let rec=Recorder(writer,sink), session=AVCaptureSession(), output=AVCaptureAudioDataOutput(), queue=DispatchQueue(label:"fm1-usb-record")
let input=try AVCaptureDeviceInput(device:device)
guard session.canAddInput(input),session.canAddOutput(output) else {fatalError("USB audio session unavailable")}
session.addInput(input);session.addOutput(output);output.setSampleBufferDelegate(rec,queue:queue)
guard writer.startWriting() else {fatalError("Writer: \(String(describing:writer.error))")}
session.startRunning()
try Data(device.localizedName.utf8).write(to:URL(fileURLWithPath:path+".ready"))
print("Recording only FoMni FM-1 USB audio for \(seconds)s → \(path)");fflush(stdout)
let deadline=Date(timeIntervalSinceNow:seconds)
while Date()<deadline && !FileManager.default.fileExists(atPath:args[3]) {RunLoop.current.run(until:Date(timeIntervalSinceNow:0.1))}
session.stopRunning();queue.sync{}
sink.markAsFinished();let done=DispatchSemaphore(value:0);writer.finishWriting{done.signal()};done.wait()
let report:[String:Any]=["device":device.localizedName,"unique_id":device.uniqueID,"samples":rec.count,"buffers_dropped":rec.dropped,"first_pts_seconds":rec.first as Any,"writer_status":writer.status.rawValue,"error":writer.error?.localizedDescription ?? "","buffers":rec.rows]
try JSONSerialization.data(withJSONObject:report,options:[.prettyPrinted,.sortedKeys]).write(to:URL(fileURLWithPath:path+".json"))
print("Finished: \(rec.count) frames, \(rec.dropped) dropped buffers, status \(writer.status.rawValue)")
if writer.status != .completed || rec.count == 0 {exit(1)}
