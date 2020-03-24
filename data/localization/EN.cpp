#include "Config.h"
#include "common/Localization.h"
#include "common/Internationalization.h"

struct LocaleEN : public Locale
{
	String GetName() const { return "English"_ascii; }

	size_t GetPluralIndex(size_t n) const
	{
		// 0: singular, 1: plural
		return n == 1 ? 0 : 1;
	}

	void Set() const
	{
		using i18n::translation;
		using i18n::pluralForm;

		pluralForm("save") = {"save"_ascii, "saves"_ascii};
		pluralForm("stamp") = {"stamp"_ascii, "stamps"_ascii};
	}

	String GetIntroText() const
	{
		return
			U"\blThe Powder Toy - Version " MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) " - https://powdertoy.co.uk, irc.freenode.net #powder\n"
			"\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\uE069\n"
			"\n"
			"\bgControl+C/V/X are Copy, Paste and cut respectively.\n"
			"\bgTo choose a material, hover over one of the icons on the right, it will show a selection of elements in that group.\n"
			"\bgPick your material from the menu using mouse left/right buttons.\n"
			"Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
			"Shift+drag will create straight lines of particles.\n"
			"Ctrl+drag will result in filled rectangles.\n"
			"Ctrl+Shift+click will flood-fill a closed area.\n"
			"Use the mouse scroll wheel, or '[' and ']', to change the tool size for particles.\n"
			"Middle click or Alt+Click to \"sample\" the particles.\n"
			"Ctrl+Z will act as Undo.\n"
			"\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength.\n"
			"The spacebar can be used to pause physics. Use 'F' to step ahead by one frame.\n"
			"Use 'S' to save parts of the window as 'stamps'. 'L' loads the most recent stamp, 'K' shows a library of stamps you saved.\n"
			"Use 'P' to take a screenshot and save it into the current directory.\n"
			"Use 'H' to toggle the HUD. Use 'D' to toggle debug mode in the HUD.\n"
			"\n"
			"Contributors: \bgStanislaw K Skowronek (Designed the original Powder Toy),\n"
			"\bgSimon Robertshaw, Skresanov Savely, cracker64, Catelite, Bryan Hoyle, Nathan Cousins, jacksonmj,\n"
			"\bgFelix Wallin, Lieuwe Mosch, Anthony Boot, Matthew \"me4502\", MaksProg, jacob1, mniip, LBPHacker\n"
			"\n"
#ifndef BETA
			"\bgTo use online features such as saving, you need to register at: \brhttps://powdertoy.co.uk/Register.html\n"
#else
			"\brThis is a BETA, you cannot save things publicly. If you are planning on publishing any saves, use the release version\n"
#endif
			;
	}

	String GetSavePublishingInfo() const
	{
		return
			U"In The Powder Toy, one can save simulations to their account in two privacy levels: Published and unpublished. You can choose which one by checking or unchecking the 'publish' checkbox. Saves are unpublished by default, so if you do not check publish nobody will be able to see your saves.\n"
			"\n"
			"\btPublished saves\bw will appear on the 'By Date' feed and will be seen by many people. These saves also contribute to your Average Score, which is displayed publicly on your profile page on the website. Publish saves that you want people to see so they can comment and vote on.\n"
			"\btUnpublished saves\bw will not be shown on the 'By Date' feed. These will not contribute to your Average Score. They are not completely private though, as anyone who knows the save id will be able to view it. You can give the save id out to show specific people the save but not allow just everyone to see it.\n"
			"\n"
			"To quickly resave a save, open it and click the left side of the split resave button to \bt'Reupload the current simulation'\bw. If you want to change the description or change the published status, you can click the right side to \bt'Modify simulation properties'\bw. Note that you can't change the name of saves; this will create an entirely new save with no comments, votes, or tags; separate from the original.\n"
			"You may want to publish an unpublished save after it is finished, or to unpublish some currently published ones. You can do this by opening the save, selecting the 'Modify simulation properties' button, and changing the published status there. You can also \btunpublish or delete saves\bw by selecting them in the 'my own' section of the browser and clicking either one of the buttons that appear on bottom.\n"
			"If a save is under a week old and gains popularity fast, it will be automatically placed on the \btfront page\bw. Only published saves will be able to get here. Moderators can also choose to promote any save onto the front page, but this happens rarely. They can also demote any save from the front page that breaks a rule or they feel doesn't belong.\n"
			"Once you make a save, you can resave it as many times as you want. A short previous \btsave history\bw is saved, just right click any save in the save browser and select 'View History' to view it. This is useful for when you accidentally save something you didn't mean to and want to go back to the old version.\n";
	}

	String GetRules() const
	{
		return
			U"\boSection S: Social and Community Rules\n"
			"\bwThere are a few rules you should follow while interacting with the community. These rules are enforced by staff members and any issues related to violations of these rules may be brought to our attention by users. This section applies to saves uploaded, comments area, forums, and other areas of the community.\n"
			"\n"
			"\bt1. Try to use proper grammar.\bw English is the official community language, but use is not required in regional or cultural groups. If you cannot write English well, we advise that you use Google Translate.\n"
			"\bt2. Do not spam.\bw There's not a one size fits all definition here, but the idea is usually obvious. In addition, the following are seen as spam and may be hidden or deleted:\n"
			   "- Posting multiple threads on the same subject. Try to combine threads on game feedback or suggestions into one thread.\n"
			   "- Bumping an old thread by replying. This is what we call 'necro' or 'necroing'. The content of the thread may be stale (fixing issues, ideas, etc). We recommend posting a new thread for an updated or more current response.\n"
			   "- Posting on a thread with '+1' or other short replies. There's no need to constantly bump a thread and make finding replies difficult. Replies are great for constructive feedback, while the '+1' button is to show your support for the content.\n"
			   "- Comments that are excessively long or gibberish. Making comments such as repeating the same letter or have little to no intended purpose, fall under this rule. Comments that are in a different language are exempt.\n"
			   "- Excessive formatting. UPPERCASE, Bold, and italics can be nice with moderate use, but please do not use them throughout the entire post.\n"
			"\bt3. Keep swearing to a minimum.\bw Comments or saves containing swearing are at risk of being deleted. This also includes swearing in other languages.\n"
			"\bt4. Refrain from uploading sexually explicit, offensive, or other inappropriate materials.\bw\n"
			   "- These include, but are not limited to: sex, drugs, racism, excessive politics, or anything that offends or insults a group of people.\n"
			   "- Reference to these topics in other languages is also prohibited. Do not attempt to bypass this rule.\n"
			   "- Posting URLs or images that violate this rule is prohibited. This includes links or text in your profile information.\n"
			"\bt5. Do not advertise third-party games, sites, or other places not related to The Powder Toy.\bw\n"
			   "- Mainly this rule is intended to prevent people going through and advertising their own games and products.\n"
			   "- Unauthorized or unofficial community gathering places, such as Discord, are prohibited.\n"
			"\bt6. Trolling is not allowed.\bw As with some rules, there's no clear definition. Users who repeatedly troll are far more likely to be banned and recieve longer bans than others.\n"
			"\bt7. Do not impersonate anyone.\bw Registering accounts with names intentionally similar to other users in our community or other online communities is prohibited.\n"
			"\bt8. Do not post about moderator decisions or issues.\bw If there is a problem regarding a ban on your account or content removal, please contact a moderator through the messages system. Otherwise, discussion about moderator actions should be avoided.\n"
			"\bt9. Avoid backseat moderating.\bw Moderators are the ones who make the decisions. Users should refrain from threatening bans or possible results from breaking a rule. If there is a possible issue or you are unsure, we recommend reporting the issue through the 'Report' button or via the messaging system on the website.\n"
			"\bt10. Condoning of breaking common laws is prohibited.\bw The jurisdiction of which country's laws applies is not clear, but there are some common ones to know. These include, but not limited to:\n"
			   "- Piracy of software, music, bagels, etc.\n"
			   "- Hacking / Stealing accounts\n"
			   "- Theft / Fraud\n"
			"\bt11. Do not stalk or harass any user.\bw This has been a growing problem in recent years by different methods, but generally these include:\n"
			   "- 'Doxing' user(s) to find where they live or their real identity\n"
			   "- Constantly messaging a user when they wish to refrain from any contact\n"
			   "- Mass downvoting saves\n"
			   "- Posting rude or unnecessary comments on someone's content (saves, forum threads, etc)\n"
			   "- Coercing a group of users to 'target' a user\n"
			   "- Personal arguments or hatred. This could be arguing in the comments or making hate saves\n"
			   "- Discrimination, in general, of people. This could be religious, ethnic, etc.\n"
			"\n"
			"\boSection G: In-Game Rules\n"
			"\bwThis section of the rules is focused on in-game actions. Though, Section S also applies in-game, the following rules are more specialized to in-game community interaction.\n"
			"\bt1. Don't claim other people's work.\bw This could be simply re-uploading another user's or utilizing large sections of saves. Derivative works are allowed, with proper usage. Should you utilize someone's work, by default you must credit the author. Unless the author has explicitly noted different usage terms, this is the standard policy. Derivative works are characterized by innovative usage and originality percentage (ie. how much is original versus someone's work?). Stolen saves will be unpublished or disabled.\n"
			"\bt2. Self-voting or vote fraud is not allowed.\bw This is defined as making multiple accounts to vote on your own saves or the saves of others. We enforce this rule strictly, therefore, you must understand that there are very few successful ban appeals. Please ensure you and other accounts are not voting from the same household. All alternate accounts will be permanently banned, the main account will be temporarily banned and any affected saves will be disabaled.\n"
			"\bt3. Asking for votes of any kind is frowned upon.\bw Saves which do this will be unpublished until the issue is fixed. Examples of such that are under this rules are:\n"
			   "- Signs that may hint at voting up or down. The signature green arrow or asking for votes goes under this rule.\n"
			   "- Gimmicks that ask for votes. These might be a total number of votes in exchange for something, like '100 votes and I'll make a better version'. This is what we define as vote farming. Any type of vote farming is not allowed.\n"
			   "- Asking for votes in return for usage of a save or for any other reason is prohibited.\n"
			"\bt4. Do not spam.\bw As mentioned earlier, there are no standards for what counts as spam. Here are some examples that may qualify as spamming:\n"
			   "- Uploading or re-uploading similar saves within a short amount of time. Don't try to circumvent the system to have your saves seen/voted by people. This includes uploading 'junk' or 'blank' saves with little to no purpose. These saves will be unpublished.\n"
			   "- Uploading text-only saves. These may be announcements or looking for help of sorts. We have the forums and comments area available for many purposes these text-only saves would serve. These saves will be removed from front page.\n"
			   "- Uploading art saves is not strictly prohibited, but may result in a front-page demotion. We like to see usage of the variety of elements in a creative manner. Lack of these factors (such as in deco-only saves) will typically result in a front-page demotion\n"
			"\bt5. Refrain from uploading sexually explicit or other inappropriate materials. These saves will be deleted and will lead to a ban.\bw\n"
			   "- These include, but are not limited to: sex, drugs, racism, excessive politics, or anything that offends or insults a group of people.\n"
			   "- Don't try to circumvent this rule. Anything that intentionally refers to these concepts/ideas by direct or indirect means falls under this rule.\n"
			   "- Reference to these topics in other languages is also prohibited. Do not attempt to bypass this rule.\n"
			   "- Posting URLs or images that violate this rule is prohibited. This includes links or text in your profile information.\n"
			"\bt6. Image plotting is strictly prohibited.\bw This includes usage of scripting or any third-party tools to plot or create a save for you. Saves using CGI will be deleted and you may receive a ban.\n"
			"\bt7. Keep logos and signs to a minimum.\bw These saves may be removed from front page. Items that this rule restricts are:\n"
			   "- Excessive logos placed\n"
			   "- Signs without intended purpose\n"
			   "- Fake update or notifications signs\n"
			   "- Linking other saves that have no related purposes\n"
			"\bt8. Do not place offtopic or inappropriate tags.\bw Tags are only there to improve search results. They should generally only be one word descriptions of the save. Sentences or subjective tags may be deleted. Inappropriate or offensive tags will likely get you banned.\n"
			"\bt9. Intentional lag inducing or crashing saves are prohibited.\bw If the majority of users are writing about the save causing crashes or lag, then the save will fall under this rule. These saves will be removed from front page or disabled.\n"
			"\bt10. Do not misuse the reporting system.\bw Sending in report reasons such as 'bad save' or gibberish wastes our time. Unless the issue pertains to a possible rule violation or community issue, please refrain from sending a report. If you think the save violates or poses a community issue, send a report anyway! Bans will never happen if you are reporting a save in good faith.\n"
			"\bt11. Do not ask for saves to be demoted or removed from the front-page.\bw Unless the save violates any rules, it will stay on the front-page. There is no exception to this rule for art saves, please do not report art either.\n"
			"\n"
			"\boSection R: Other\n"
			"\bwModerators may interpret these rules as they see fit. Not all rules are equal, some are enforced less than others. Moderators make the final decision on what is and isn't against the rules, but we have made our best effort here to cover all unwanted behavior here. Notice will be posted in this thread whenever the rules are updated.\n"
			"\n"
			"Violation of these rules may result in removal of posts / comments, unpublishing or disabling saves, removing saves from front page, or in more extreme cases, a temporary or permanent ban. There are various manual and automated measures in place to enforce these rules. The severity and resulting decisions may not be consistent between moderators.\n"
			"\n"
			"If you have any questions about what is and isn't against the rules, feel free to contact a moderator.";
	}
};

Locale const &Locale_EN = LocaleEN{};
