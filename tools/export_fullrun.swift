// Release portability/documentation changes: 2026-10-17; firmware behavior and assets unchanged.
// SPDX-License-Identifier: GPL-3.0-only
// Full recording at original received timestamps; streamed pixels keep RAM bounded.
import AVFoundation
import Foundation
import CoreVideo
let dir=CommandLine.arguments[1], result=dir+"/playthrough.mov", audioPath=dir+"/usb-audio.mov"
let temp=URL(fileURLWithPath:dir+"/video.mov"), stdinFile=FileHandle.standardInput
func readExact(_ n:Int, eof:Bool=false)throws->Data? {var d=Data();while d.count<n {let p=try stdinFile.read(upToCount:n-d.count) ?? Data();if p.isEmpty {if d.isEmpty && eof{return nil};throw NSError(domain:"Incomplete frame stream",code:1)};d.append(p)};return d}
let writer=try AVAssetWriter(outputURL:temp,fileType:.mov)
let input=AVAssetWriterInput(mediaType:.video,outputSettings:[AVVideoCodecKey:AVVideoCodecType.h264,AVVideoWidthKey:240,AVVideoHeightKey:240,AVVideoCompressionPropertiesKey:[AVVideoAverageBitRateKey:2000000]])
let adaptor=AVAssetWriterInputPixelBufferAdaptor(assetWriterInput:input,sourcePixelBufferAttributes:[kCVPixelBufferPixelFormatTypeKey as String:kCVPixelFormatType_32BGRA,kCVPixelBufferWidthKey as String:240,kCVPixelBufferHeightKey as String:240,kCVPixelBufferIOSurfacePropertiesKey as String:[:]])
writer.add(input);guard writer.startWriting() else{fatalError("Cannot start video writer: \(String(describing:writer.error))")};writer.startSession(atSourceTime:.zero)
var first:Double?,last=0.0,count=0
while let h=try readExact(8,eof:true){let t=h.withUnsafeBytes{$0.loadUnaligned(as:Double.self)};if first==nil{first=t};guard t>=last else{fatalError("Out-of-order timestamp")};last=t
 let data=try readExact(230400)!
 while !input.isReadyForMoreMediaData {if writer.status == .failed{fatalError("Video writer failed")};Thread.sleep(forTimeInterval:0.001)}
 var pb:CVPixelBuffer?;guard CVPixelBufferPoolCreatePixelBuffer(nil,adaptor.pixelBufferPool!,&pb)==kCVReturnSuccess else{fatalError("Pixel allocation failed")}
 CVPixelBufferLockBaseAddress(pb!,[]);let base=CVPixelBufferGetBaseAddress(pb!)!,stride=CVPixelBufferGetBytesPerRow(pb!)
 data.withUnsafeBytes{ptr in for y in 0..<240{memcpy(base.advanced(by:y*stride),ptr.baseAddress!.advanced(by:y*960),960)}};CVPixelBufferUnlockBaseAddress(pb!,[])
 guard adaptor.append(pb!,withPresentationTime:CMTime(seconds:t-first!,preferredTimescale:1000000)) else{fatalError("Video frame rejected")};count+=1
}
guard let first=first,count>0 else{fatalError("No received frames")}
let capture=try JSONSerialization.jsonObject(with:Data(contentsOf:URL(fileURLWithPath:dir+"/capture.json"))) as! [String:Any]
let end=capture["end_host_seconds"] as! Double
writer.endSession(atSourceTime:CMTime(seconds:max(last-first+0.001,end-first),preferredTimescale:1000000));input.markAsFinished();let done=DispatchSemaphore(value:0);writer.finishWriting{done.signal()};done.wait();guard writer.status == .completed else{fatalError("Video finalization failed")}
let composition=AVMutableComposition(),video=AVURLAsset(url:temp),audio=AVURLAsset(url:URL(fileURLWithPath:audioPath))
guard let v=video.tracks(withMediaType:.video).first,let a=audio.tracks(withMediaType:.audio).first else{fatalError("Missing video or USB audio")}
let meta=try JSONSerialization.jsonObject(with:Data(contentsOf:URL(fileURLWithPath:audioPath+".json"))) as! [String:Any]
let firstAudio=meta["first_pts_seconds"] as! Double,offset=first-firstAudio
let source=max(0,offset),destination=max(0,-offset),length=min(audio.duration.seconds-source,video.duration.seconds-destination)
guard length>0 else{fatalError("Audio/video clocks do not overlap")}
try composition.addMutableTrack(withMediaType:.video,preferredTrackID:kCMPersistentTrackID_Invalid)!.insertTimeRange(CMTimeRange(start:.zero,duration:video.duration),of:v,at:.zero)
try composition.addMutableTrack(withMediaType:.audio,preferredTrackID:kCMPersistentTrackID_Invalid)!.insertTimeRange(CMTimeRange(start:CMTime(seconds:source,preferredTimescale:44100),duration:CMTime(seconds:length,preferredTimescale:44100)),of:a,at:CMTime(seconds:destination,preferredTimescale:44100))
let exporter=AVAssetExportSession(asset:composition,presetName:AVAssetExportPresetPassthrough)!;exporter.outputURL=URL(fileURLWithPath:result);exporter.outputFileType = .mov
let exported=DispatchSemaphore(value:0);exporter.exportAsynchronously{exported.signal()};exported.wait();guard exporter.status == .completed else{fatalError("Export failed: \(String(describing:exporter.error))")}
let output=AVURLAsset(url:URL(fileURLWithPath:result));guard output.tracks(withMediaType:.video).count==1 && output.tracks(withMediaType:.audio).count==1 else{fatalError("Export missing a track")}
let report:[String:Any]=["output":result,"actual_device_frames":count,"duration_seconds":output.duration.seconds,"audio_start_offset_seconds":offset,"audio_duration_seconds":length,"video_tracks":1,"audio_tracks":1,"scope":"All received device snapshots, take-frozen matching renderer, direct FM-1 USB audio aligned by host timestamps. No simulation, gameplay commands or invented motion; LCD/USB latency is not calibrated."]
try JSONSerialization.data(withJSONObject:report,options:[.prettyPrinted,.sortedKeys]).write(to:URL(fileURLWithPath:result+".json"));print("Exported \(count) frames to \(result)")
