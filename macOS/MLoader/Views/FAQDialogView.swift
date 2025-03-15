// Copyright (c) 2025 mlogic1
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.


import SwiftUI

struct FAQDialogView: View {
	@Environment(\.presentationMode) var presentationMode
	
	private let faqItems: [FAQItem] = [
		FAQItem(question: "1. My headset is connected to the computer but I don't see it on the list of devices", answer: "• Your headset needs to have developer mode enabled.\n• You need to Allow USB debugging after you connect your headset\n• You need to Allow access to files in the notifications menu after you connect your headset"),
		FAQItem(question: "2. My headset is on the list but its status is 'Unauthorized'", answer: "You need to Allow USB debugging after you connect your headset"),
		FAQItem(question: "3. How do I enable developer mode?", answer: "Please see this guide: https://wiki.vrpirates.club/en/Howto/enabling-dev-mode on how to enable developer mode"),
		FAQItem(question: "4. How do I allow USB debugging?", answer: "When you plug in your headset to your computer, the headset will show a popup asking you if you 'Allow USB debugging?'. Click 'Allow' or 'Always allow from this computer'. If you missed it, simply reconnect your headset and it will pop-up again"),
		FAQItem(question: "5. How do I allow access to files?", answer: "When you plug in your headset to your computer, the headset will show a small popup saying 'USB Detected - Click on this notification to allow connected device to access files'. Simply click it. If you missed it it will also be in the Notifications menu.")
	]
	
	var body: some View {
		VStack{
			Text("Frequently Asked Questions")
						.font(.title)
						.padding()
			ScrollView {
				VStack(alignment: .leading, spacing: 10) {
					ForEach(faqItems) { item in
						VStack(alignment: .leading) {
							Text(item.question)
								.font(.title2)
							Text(item.detectedLinksAttributedString)
								.font(.title3)
								.foregroundColor(.secondary)
						}
						Divider()
					}
				}
				.padding()
			}
			Button("Close") {
				presentationMode.wrappedValue.dismiss()
			}
			.padding()
		}
		.frame(width: 700, height: 550)
    }
}

struct FAQDialogView_Previews: PreviewProvider {
    static var previews: some View {
        FAQDialogView()
    }
}

struct FAQItem: Identifiable {
	let id = UUID()
	let question: String
	let answer: String
	
	var detectedLinksAttributedString: AttributedString {
		var attributed = AttributedString(answer)
		
		// Detect URLs in the answer string
		let urlDetector = try? NSDataDetector(types: NSTextCheckingResult.CheckingType.link.rawValue)
		let nsRange = NSRange(answer.startIndex..<answer.endIndex, in: answer)

		urlDetector?.enumerateMatches(in: answer, options: [], range: nsRange) { match, _, _ in
			if let match = match, let url = match.url {
				if let range = Range(match.range, in: answer) {
					// Convert String range to AttributedString range
					let attributedRange = attributed.range(of: String(answer[range]))
					if let attributedRange = attributedRange {
						attributed[attributedRange].link = url
						attributed[attributedRange].foregroundColor = .blue
					}
				}
			}
		}

		return attributed
	}
}

