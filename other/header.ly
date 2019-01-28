\version "2.18"

#(set-default-paper-size "a6landscape")

\paper {
%   remove first line indent
  indent = 0\cm
%   remove page number
  print-page-number = ##f
}

\header {
%   remove footer
  tagline = ""
}

\score {
  
  \layout {
    \context {
      \Score
      \remove "Bar_number_engraver"
    }
    \context {
      \Staff
      \remove Time_signature_engraver
%      \hide BarLine
%      \hide SpanBar
    }
  }
  
{
\time 32/4

